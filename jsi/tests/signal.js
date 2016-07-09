/*
=!EXPECTSTART!=
[ "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGCLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR", "SIGSYS" ]
MYSIG
=!EXPECTEND!=
*/

puts(Signal.names());
function mysig() {
  puts("MYSIG");
  exit(0);
}

i = getpid();
//Signal.handle('USR1');
Signal.callback(mysig,'USR1');

Signal.kill(getpid(),'USR1');
while (true) {
   Event.update(1000);
}
