DisplayURL is based on the Apple, inc. example:
http://developer.apple.com/samplecode/DisplayURL/index.html#//apple_ref/doc/uid/DTS10003783

It has been modified to build using OpenCFLite.  Consequently,
the file system routines are currently disabled.

DisplayURL accepts a URL as an input string, and parses it into
its various components using CFURL routines.

DisplayURL [-h] [-u <url>]
        DisplayURL parses a URL's components
        -h       Shows this help message.
        -u <url> The URL to parse.

The sample also illustrates how to use the CFURLGetByteRangeForComponent API.  As an example you can see how CFURLGetByteRangeForComponent deconstructs the following URL into its components.

> DisplayURL -u "scheme://user:pass@host:1/path/path2/file.html;params?query#fragment"
url: "scheme://user:pass@host:1/path/path2/file.html;params?query#fragment"
kCFURLComponentScheme: "scheme" including separators: "scheme://"
kCFURLComponentNetLocation: "user:pass@host:1" including separators: "://user:pass@host:1"
kCFURLComponentPath: "/path/path2/file.html" including separators: "/path/path2/file.html;"
kCFURLComponentResourceSpecifier: "params?query#fragment" including separators: ";params?query#fragment"
kCFURLComponentUser: "user" including separators: "://user:"
kCFURLComponentPassword: "pass" including separators: ":pass@"
kCFURLComponentUserInfo: "user:pass" including separators: "://user:pass@"
kCFURLComponentHost: "host" including separators: "@host:"
kCFURLComponentPort: "1" including separators: ":1"
kCFURLComponentParameterString: "params" including separators: ";params?"
kCFURLComponentQuery: "query" including separators: "?query#"
kCFURLComponentFragment: "fragment" including separators: "#fragment"

