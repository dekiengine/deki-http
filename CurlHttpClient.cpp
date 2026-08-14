#include "CurlHttpClient.h"
#include "DekiLogSystem.h"

#ifndef _WIN32

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

// Marker that separates the response body from the status code curl appends
// via --write-out. A body could legitimately end in digits or newlines, so we
// split on the LAST occurrence of a token that will not appear in real content.
constexpr const char* kStatusMarker = "\n__deki_http_status__:";

/**
 * Run curl with an explicit argv (no shell) and collect stdout.
 *
 * @param args      argv for curl, excluding argv[0]
 * @param stdinData optional request body piped to curl's stdin
 * @param outStdout captured stdout
 * @return true if curl was launched and exited 0
 */
bool RunCurl(const std::vector<std::string>& args,
             const std::string*              stdinData,
             std::string&                    outStdout)
{
    outStdout.clear();

    int outPipe[2] = {-1, -1};
    int inPipe[2]  = {-1, -1};

    if (pipe(outPipe) != 0)
    {
        DEKI_LOG_ERROR("[deki-http] pipe() failed: %s", strerror(errno));
        return false;
    }
    if (stdinData && pipe(inPipe) != 0)
    {
        DEKI_LOG_ERROR("[deki-http] pipe() failed: %s", strerror(errno));
        close(outPipe[0]); close(outPipe[1]);
        return false;
    }

    // Build argv. Pointers reference `args`, which outlives the call.
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("curl"));
    for (const auto& a : args)
        argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);

    const pid_t pid = fork();
    if (pid < 0)
    {
        DEKI_LOG_ERROR("[deki-http] fork() failed: %s", strerror(errno));
        close(outPipe[0]); close(outPipe[1]);
        if (stdinData) { close(inPipe[0]); close(inPipe[1]); }
        return false;
    }

    if (pid == 0)
    {
        // Child. Only async-signal-safe calls from here to execvp.
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(outPipe[1], STDERR_FILENO);
        close(outPipe[0]);
        close(outPipe[1]);

        if (stdinData)
        {
            dup2(inPipe[0], STDIN_FILENO);
            close(inPipe[0]);
            close(inPipe[1]);
        }

        execvp("curl", argv.data());
        _exit(127);  // exec failed — curl not installed or not on PATH
    }

    // Parent.
    close(outPipe[1]);

    if (stdinData)
    {
        close(inPipe[0]);
        // curl can exit early (bad URL, timeout) leaving nobody reading this
        // pipe; the default SIGPIPE would kill the editor rather than the
        // write just failing. Ignore it for the duration of the write.
        struct sigaction ignore{}, previous{};
        ignore.sa_handler = SIG_IGN;
        sigemptyset(&ignore.sa_mask);
        sigaction(SIGPIPE, &ignore, &previous);

        size_t written = 0;
        while (written < stdinData->size())
        {
            const ssize_t n = write(inPipe[1],
                                    stdinData->data() + written,
                                    stdinData->size() - written);
            if (n <= 0)
            {
                if (n < 0 && errno == EINTR) continue;
                break;
            }
            written += static_cast<size_t>(n);
        }

        sigaction(SIGPIPE, &previous, nullptr);
        close(inPipe[1]);
    }

    char buffer[4096];
    ssize_t n;
    while ((n = read(outPipe[0], buffer, sizeof(buffer))) != 0)
    {
        if (n < 0)
        {
            if (errno == EINTR) continue;
            break;
        }
        outStdout.append(buffer, static_cast<size_t>(n));
    }
    close(outPipe[0]);

    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}

    if (!WIFEXITED(status))
    {
        DEKI_LOG_WARNING("[deki-http] curl terminated abnormally");
        return false;
    }

    const int code = WEXITSTATUS(status);
    if (code == 127)
    {
        DEKI_LOG_ERROR("[deki-http] 'curl' not found on PATH — install curl to enable "
                       "HTTP on this platform");
        return false;
    }
    if (code != 0)
    {
        // curl writes its diagnostic to stderr, which we merged into stdout.
        DEKI_LOG_WARNING("[deki-http] curl exited %d: %s", code, outStdout.c_str());
        return false;
    }
    return true;
}

/// Split curl's combined output into body + status code.
IDekiHttpClient::Response SplitStatus(const std::string& raw)
{
    IDekiHttpClient::Response r;

    const size_t marker = raw.rfind(kStatusMarker);
    if (marker == std::string::npos)
    {
        // No marker: curl produced nothing usable (already logged by RunCurl).
        r.body = raw;
        return r;
    }

    r.body = raw.substr(0, marker);
    const std::string codeText = raw.substr(marker + strlen(kStatusMarker));
    r.status = std::atoi(codeText.c_str());
    if (r.status == 0)
        r.status = -1;
    return r;
}

/// Flags shared by every request.
void AppendCommonArgs(std::vector<std::string>& args,
                      const IDekiHttpClient::HeaderList& headers,
                      uint32_t timeoutMs)
{
    args.push_back("-sS");                  // quiet, but still report errors
    args.push_back("-L");                   // follow redirects
    args.push_back("--max-time");
    args.push_back(std::to_string((timeoutMs + 999) / 1000));  // curl wants seconds
    args.push_back("--write-out");
    args.push_back(std::string(kStatusMarker) + "%{http_code}");

    for (const auto& [name, value] : headers)
    {
        args.push_back("-H");
        args.push_back(name + ": " + value);
    }
}

}  // namespace

std::string CurlHttpClient::FetchUrl(const std::string& url)
{
    const Response r = Get(url);
    // Legacy contract: empty string on any failure, including non-2xx.
    if (r.status < 200 || r.status > 299)
        return "";
    return r.body;
}

IDekiHttpClient::Response CurlHttpClient::Get(const std::string& url,
                                              const HeaderList&  headers,
                                              uint32_t           timeoutMs)
{
    std::vector<std::string> args;
    AppendCommonArgs(args, headers, timeoutMs);
    args.push_back("--");     // no more flags; a URL starting with '-' is still a URL
    args.push_back(url);

    std::string raw;
    if (!RunCurl(args, nullptr, raw))
        return {};            // status -1, empty body

    return SplitStatus(raw);
}

IDekiHttpClient::Response CurlHttpClient::PostJson(const std::string& url,
                                                   const std::string& body,
                                                   const HeaderList&  headers,
                                                   uint32_t           timeoutMs)
{
    std::vector<std::string> args;
    AppendCommonArgs(args, headers, timeoutMs);
    args.push_back("-H");
    args.push_back("Content-Type: application/json");
    args.push_back("-X");
    args.push_back("POST");
    args.push_back("--data-binary");
    args.push_back("@-");     // read the body from stdin, so it needs no escaping
    args.push_back("--");
    args.push_back(url);

    std::string raw;
    if (!RunCurl(args, &body, raw))
        return {};

    return SplitStatus(raw);
}

#else  // _WIN32

// Windows uses WinHttpClient; this file compiles to nothing there so the module
// keeps one source list across platforms.

std::string CurlHttpClient::FetchUrl(const std::string& /*url*/)
{
    DEKI_LOG_ERROR("[deki-http] CurlHttpClient is not used on Windows");
    return "";
}

IDekiHttpClient::Response CurlHttpClient::Get(const std::string&, const HeaderList&, uint32_t)
{
    return {};
}

IDekiHttpClient::Response CurlHttpClient::PostJson(const std::string&, const std::string&,
                                                   const HeaderList&, uint32_t)
{
    return {};
}

#endif // _WIN32
