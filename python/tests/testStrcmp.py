from python.comm.socket.utils import strcmp, strToInt


def test():
    assert (strToInt("8348\0") == 8348)
    assert (strcmp("asd", "asd") == 0)
    assert (strcmp("asd", "asd\0") == 0)
    assert (strcmp("asd\0", "asd") == 0)
    assert (strcmp("asd\0", "asd\0") == 0)
    assert (strcmp("asd\0", "asd" + chr(1)) == -1)
    assert (strcmp("asd" + chr(2), "asd") == 2)
    assert (strcmp("asd" + chr(2), "asd" + chr(1)) == 1)
    assert (strcmp("bsd", "asd") == 1)
    assert (strcmp("asd", "bsd") == -1)


if __name__ == "__main__":
    test()
