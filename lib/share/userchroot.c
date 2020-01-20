/**
 * A simple program to aid in launching chroot/--rbound programs
 * using the original user credentials. This program is supposed to
 * be used with suid-bit set (chmod u+s).
 *
 * Copyright 2007 Aleksandr Koltsoff (czr@iki.fi)
 * Released under the GNU General Public License version 2 (GPL).
 * Please see provided COPYING file or http://www.gnu.org/licenses/gpl.txt
 *
 *
 * The objective of this program is to:
 * - chroot (argv[1])
 * - change working directory to (argv[2])
 * - switch user credentials back to original (since this was
 *   run via suid-bit)
 * - exec the target program (argv[3]) (we use direct exec)
 *
 * rest of the argv is passed to the target program directly.
 * environment is passed to the target program (without modifications)
 *
 * Parameters:
 * - argv[1]: new-root (must start with abs-path)
 * - argv[2]: new working dir, relative to post-chroot
 * - argv[3]: executable name (related to new working directory) to execve
 * - argv[4+ if any]: additional parameters passed to target executable
 *
 * Never returns success exit code (0), but will return error codes
 * if checks will fail before getting to the execve-part. In that case
 * errors will be print out on stderr.
 *
 * Exit codes:
 * 1: problem with command line arguments (usage printed)
 * 2: no root-privs (won't even attempt to chroot)
 * 3: chroot target does not exist or cannot be accessed with real
 *    UID/GID (non-root) (tested with access(2))
 * 4: chroot-syscall fails
 * 5: cd / fails (post chroot)
 * 6: dropping privileges failed
 * 7: no X_OK on the target (just before execve)
 * 8: execve failed
 * 9: somehow execve returned even if it didn't fail (impossible)
 */

#include <unistd.h> // {g,s}et{e,}{u,g}id(), execve and other friends
#include <sys/types.h>
#include <stdio.h>

#define PROGNAME "userchroot"

#define DEBUG (0)

// we don't modify our environment on the way to destination program
extern char** environ;

int main(int argc, char** argv) {

  // we'll use these to check whether dropping privs succeeds
  gid_t origGid;
  uid_t origUid;

  if (DEBUG) {
    printf(PROGNAME ": starting (pid=%u)\n", (unsigned)getpid());
  }

  // check for params
  if (argc < 4) {
    fprintf(stderr, PROGNAME
      ": USAGE: new-root new-cwd exec-name [exec-params]\n");
    return 1;
  }

  if (DEBUG) {
    // euid should be root, uid should be the original (assuming u+s)
    printf("uid=%u euid=%u\n",
      (unsigned)getuid(), (unsigned)geteuid());
    // if egid != gid, we restore it as well (later)
    printf("gid=%u egid=%u\n",
      (unsigned)getgid(), (unsigned)getegid());
  }

  // check that we have the proper creds (need root for chroot)
  if ((geteuid() != 0) && (getegid() != 0)) {
    fprintf(stderr, PROGNAME ": no root privs (suid missing?)\n");
    return 2;
  }
  // check for X_OK for target dir (blah, this uses real UID/GID,
  // not effective. but we can live with it
  if (access(argv[1], X_OK) != 0) {
    perror(PROGNAME ": real UID/GID cannot access new root");
    return 3;
  }
  // do the chroot
  if (chroot(argv[1]) != 0) {
    perror(PROGNAME ": failed to chroot");
    return 4;
  }
  // change working directory to /
  if (chdir(argv[2]) != 0) {
    perror(PROGNAME ": failed to switch working directory");
    return 5;
  }
  if (DEBUG) printf("Restoring privileges\n");
  // restore privileges (drop root)
  origUid = getuid();
  origGid = getgid();
  setegid(origGid);
  seteuid(origUid);
  // check that the switch was ok
  // we do not allow programs to run without the drop being
  // successful as this would possibly run the program
  // using root-privs, when that is not what we want
  if ((getegid() != origGid) || (geteuid() != origUid)) {
    fprintf(stderr, PROGNAME ": Failed to drop privileges, aborting\n");
    return 6;
  }

  // aids in debugging problematic cases
  if (DEBUG) {
    printf("uid=%u euid=%u\n", (unsigned)getuid(), (unsigned)geteuid());
    // if egid != gid, we restore it as well
    printf("gid=%u egid=%u\n", (unsigned)getgid(), (unsigned)getegid());
  }

  // verify that it's ok for us to X_OK the target executable
  if (access(argv[3], X_OK) != 0) {
    perror(PROGNAME ": target missing?");
    return 7;
  }

  // the command line that the target will get is argv[3] >.
  // we also use argv[3] as the executable name to launch.
  if (execve(argv[3], &argv[3], environ) != 0) {
    perror(PROGNAME ": failed to execve");
    return 8;
  }

  // we never should get here.
  return 9;
}
