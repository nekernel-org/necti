
static BOOL kKeepRunning = false;

#ifdef LD_NEKERNEL_DEBUGGER
static LibDebugger::NeKernel::NeKernelContract kKernelDebugger;
#else
static LibDebugger::POSIX::POSIXMachContract kDebugger;
#endif

static LibDebugger::ProcessID kPID           = 0L;
static LibDebugger::CAddress  kActiveAddress = nullptr;
static std::string            kPath          = "";

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

#define kStdOut (std::cout << kRed << "dbg: " << kWhite)