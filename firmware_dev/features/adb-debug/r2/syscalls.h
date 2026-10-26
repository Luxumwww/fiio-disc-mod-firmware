/* 最小 MIPS o32 系统调用封装（无 libc）。编号逐项取自目标 ABI 表，不用"4000+通用编号"概括。
 *
 * 错误约定（Codex 审核 R-5a，已按 MIPS UAPI 修正）：
 *   MIPS 的系统调用失败 **不是** 用 v0 < 0 表示，而是 **$a3 != 0 表示失败**，
 *   此时 $v0 里放的是正 errno。
 *   因此每个封装都必须把 $a3 声明为输出，并统一归一化成负 errno。
 *   缺了这一条时，ENOENT(2) 会被读成"成功返回 fd=2"——本项目真的踩过这个坑
 *   （表现为 open 返回 2、随后写入落到 stderr）。
 *
 * 另外必须声明系统调用会破坏的寄存器，否则编译器可能把活跃值留在 $8-$15 等寄存器里。
 *
 * 说明：修好上述两点之后，封装在 inline / noinline 与 -O2 / -O0 四种组合下行为一致，
 * 因此**不需要** noinline。此前观察到的"内联导致崩溃"是错误 ABI 声明的症状，不是编译器缺陷。
 */
#ifndef DISC_SYSCALLS_H
#define DISC_SYSCALLS_H

/* 共享头里的部分 helper 只在某一个程序里用到；显式标记避免无意义的未使用警告。 */
#define DISC_UNUSED __attribute__((unused))

#define DISC_NR_EXIT 4001
#define DISC_NR_WRITE 4004
#define DISC_NR_CLOCK_GETTIME 4263
#define DISC_NR_OPEN 4005
#define DISC_NR_CLOSE 4006
#define DISC_NR_READ 4003
#define DISC_NR_IOCTL 4054

#define DISC_CLOCK_MONOTONIC 1

/* MIPS o32 的 errno（负值形式由封装统一给出） */
#define DISC_EPERM 1
#define DISC_ENOENT 2
#define DISC_EINTR 4
#define DISC_EIO 5
#define DISC_EBADF 9
#define DISC_EINVAL 22

/* 文件标志：取自 MIPS UAPI asm/fcntl.h。
 * 注意 O_CREAT 在 MIPS 上是 0x0100(256)，不是通用 0x40(64)——本项目踩过这个坑。 */
#define DISC_O_RDONLY 0x0000
#define DISC_O_WRONLY 0x0001
#define DISC_O_NONBLOCK 0x0080
#define DISC_O_CREAT 0x0100
#define DISC_O_TRUNC 0x0200

/* 系统调用破坏的寄存器：$1($at) $3($v1) $8-$15 $24-$25 $31($ra)。 */
#define DISC_SYSCALL_CLOBBERS "memory", "$1", "$3", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31"

/* 返回 0 或负 errno。 */
static long disc_sys3(long n, long a, long b, long c) {
  register long v0 __asm__("$2") = n;
  register long a0 __asm__("$4") = a;
  register long a1 __asm__("$5") = b;
  register long a2 __asm__("$6") = c;
  register long a3 __asm__("$7");
  __asm__ volatile("syscall"
                   : "+r"(v0), "=r"(a3)
                   : "r"(a0), "r"(a1), "r"(a2)
                   : DISC_SYSCALL_CLOBBERS);
  return a3 == 0 ? v0 : -v0;
}

static long disc_sys1(long n, long a) {
  register long v0 __asm__("$2") = n;
  register long a0 __asm__("$4") = a;
  register long a3 __asm__("$7");
  __asm__ volatile("syscall"
                   : "+r"(v0), "=r"(a3)
                   : "r"(a0)
                   : DISC_SYSCALL_CLOBBERS);
  return a3 == 0 ? v0 : -v0;
}

static long disc_strlen(const char *s) {
  long n = 0;
  while (s[n]) n++;
  return n;
}

/* 写满 length 字节：处理短写与 EINTR，返回已写字节数或负 errno。
 * 只调用一次 write 就认为成功是错的（Codex 审核 R-6 第 3 条）。 */
static long disc_write_all(long fd, const char *buffer, long length) {
  long done = 0;
  while (done < length) {
    long rc = disc_sys3(DISC_NR_WRITE, fd, (long)(buffer + done), length - done);
    if (rc < 0) {
      if (rc == -DISC_EINTR) continue;
      return rc;
    }
    if (rc == 0) return -DISC_EIO;
    done += rc;
  }
  return done;
}

/* 文本输出走 stderr(2)；stdout(1) 留给二进制帧数据。 */
static void disc_write(const char *s) {
  (void)disc_write_all(2, s, disc_strlen(s));
}

/* 二进制帧输出走 stdout。返回已写字节数或负 errno，由调用者判断。 */
DISC_UNUSED static long disc_emit(const void *buffer, long length) {
  return disc_write_all(1, (const char *)buffer, length);
}

static long disc_open_read(const char *path) {
  return disc_sys3(DISC_NR_OPEN, (long)path, DISC_O_RDONLY, 0);
}

static long disc_open_read_nonblock(const char *path) {
  return disc_sys3(DISC_NR_OPEN, (long)path, DISC_O_RDONLY | DISC_O_NONBLOCK, 0);
}

static long disc_read(long fd, void *buffer, long length) {
  return disc_sys3(DISC_NR_READ, fd, (long)buffer, length);
}

static long disc_close(long fd) {
  return disc_sys1(DISC_NR_CLOSE, fd);
}

static long disc_ioctl(long fd, unsigned long request, void *argument) {
  return disc_sys3(DISC_NR_IOCTL, fd, (long)request, (long)argument);
}

static void disc_exit(int code) {
  (void)disc_sys1(DISC_NR_EXIT, code);
  for (;;) { }
}

/* 无符号 32 位十进制输出（o32 的 long 就是 32 位，函数名如实标注）。 */
static void disc_write_u32(unsigned long value) {
  char buf[24];
  int pos = 23;
  buf[pos] = 0;
  if (value == 0) {
    buf[--pos] = '0';
  } else {
    while (value > 0 && pos > 0) {
      buf[--pos] = (char)('0' + (int)(value % 10UL));
      value /= 10UL;
    }
  }
  (void)disc_write_all(2, buf + pos, disc_strlen(buf + pos));
}

/* key=value 一行；负值带 '-'，便于区分失败与失败原因。 */
static void disc_report(const char *key, long value) {
  disc_write(key);
  if (value < 0) {
    disc_write("-");
    disc_write_u32((unsigned long)(-value));
  } else {
    disc_write_u32((unsigned long)value);
  }
  disc_write("\n");
}

/* 无符号 32 位 key=value。FNV 校验值可能带最高位，必须按无符号打印，
 * 否则日志里出现负数、主机算出的却总是无符号（Codex 审核时正是抓到这里）。 */
static void disc_report_u32(const char *key, unsigned long value) {
  disc_write(key);
  disc_write_u32(value);
  disc_write("\n");
}

/* 文本形式的取值（例如 ok / unavailable）。 */
static void disc_report_text(const char *key, const char *value) {
  disc_write(key);
  disc_write(value);
  disc_write("\n");
}

#define DISC_NR_NANOSLEEP 4166

/* 短睡眠：轮询设备时避免 100% 占用 CPU（nanosleep 的 timespec 也是两个 32 位字段）。 */
static void disc_sleep_ms(long ms) {
  struct disc_sleep_ts { long sec; long nsec; } ts;
  ts.sec = ms / 1000;
  ts.nsec = (ms % 1000) * 1000000L;
  (void)disc_sys3(DISC_NR_NANOSLEEP, (long)&ts, 0, 0);
}

/* time32 的 timespec 布局：两个 32 位字段。 */
struct disc_timespec {
  long tv_sec;
  long tv_nsec;
};

/* 返回 0 或负 errno。 */
static long disc_clock_gettime(long clock_id, struct disc_timespec *ts) {
  return disc_sys3(DISC_NR_CLOCK_GETTIME, clock_id, (long)ts, 0);
}

/* timespec 是否合法：失败路径上可能根本没人写过它，必须先判断再使用。
 * nsec 必须在 [0, 1e9) 内，否则差值算法没有意义（Codex 审核：失败调用后继续读结构）。 */
DISC_UNUSED static int disc_timespec_valid(const struct disc_timespec *ts) {
  return ts->tv_sec >= 0 && ts->tv_nsec >= 0 && ts->tv_nsec < 1000000000L;
}

/* 两个 timespec 之间的微秒差。
 *
 * 不做"绝对单调时钟 -> long 微秒"的换算：o32 的 long 是 32 位，
 * tv_sec * 1000000 在 2147.48 秒（约 35.8 分钟）就溢出，而有符号溢出不是合法的环绕算法
 * （Codex 审核 R-5b）。只对**差值**做算术，合理时长内不会接近 2^31 微秒。
 */
static long disc_elapsed_us(const struct disc_timespec *from, const struct disc_timespec *to) {
  long secs = to->tv_sec - from->tv_sec;
  long nsecs = to->tv_nsec - from->tv_nsec;
  if (nsecs < 0) {
    nsecs += 1000000000L;
    secs -= 1;
  }
  return secs * 1000000L + nsecs / 1000L;
}

/* FNV-1a 32 位：主机据此独立校验帧内容就是程序算出来的那一帧。 */
DISC_UNUSED static unsigned long disc_fnv1a(const unsigned char *data, unsigned long len) {
  unsigned long hash = 2166136261UL;
  for (unsigned long i = 0; i < len; i++) {
    hash ^= (unsigned long)data[i];
    hash = (hash * 16777619UL) & 0xFFFFFFFFUL;
  }
  return hash;
}

#endif
