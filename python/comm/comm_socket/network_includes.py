import platform
import errno

os = platform.system()

connection_refused = [
    errno.ECONNREFUSED,
    errno.ENETUNREACH,
]
connection_errors = [
    errno.ECONNRESET,
    errno.ESHUTDOWN,
    errno.EPIPE
]
timeout_errors = [
    errno.ETIMEDOUT,
    errno.EAGAIN,
    errno.EWOULDBLOCK,
]
memory_size_too_large = [
    errno.EMSGSIZE,
]

if os == "Windows":
    connection_refused += [
        errno.WSAENETUNREACH,
        errno.WSAECONNREFUSED
    ]
    connection_errors += [
        errno.WSAECONNRESET,
        errno.WSAESHUTDOWN,
    ]
    timeout_errors += [
        errno.WSAETIMEDOUT,
        errno.WSAEWOULDBLOCK,
    ]
    memory_size_too_large += [
        errno.WSAEMSGSIZE,
    ]
