from __future__ import print_function, division, with_statement, generators, generator_stop, annotations

import numpy as np
import re
import sys
from numbers import Number
from typing import List, Tuple


def strToCStr(s: str or bytes):
    if isinstance(s, bytes):
        to_find = 0
    else:
        to_find = "\0"
    index = s.find(to_find)
    return s if index == -1 else s[:index]


def strToInt(s: str or bytes):
    return int(strToCStr(s))


def strcmp(s1: str or bytes, s2: str or bytes):
    if isinstance(s1, str):
        s1 = bytes(s1, "ascii")
    if isinstance(s2, str):
        s2 = bytes(s2, "ascii")
    i = 0
    s1_len = len(s1)
    s2_len = len(s2)
    while True:
        if i == s1_len:
            return 0 if i == s2_len else -s2[i]
        if i == s2_len:
            return 0 if i == s1_len else s1[i]
        if s1[i] != s2[i]:
            return s1[i] - s2[i]
        i += 1


def memcpy(destBuffer: bytes or bytearray, destStart: int, srcBuffer: bytes, srcStart: int, size: int) -> bytes:
    """
    initSize = len(destBuffer)
    newBuffer = destBuffer[:destStart] + srcBuffer[srcStart:srcStart + size] + destBuffer[destStart + size:]
    newSize = len(newBuffer)
    assert (initSize == newSize), "{} vs. {}".format(initSize, newSize)
    """
    if size == 0:
        return destBuffer
    if isinstance(destBuffer, bytearray):
        destBuffer[destStart:destStart + size] = srcBuffer[srcStart:srcStart + size]
        return destBuffer
    return destBuffer[:destStart] + srcBuffer[srcStart:srcStart + size] + destBuffer[destStart + size:]


def memset(destBuffer: bytes or bytearray, destStart: int, value: int, size: int) -> bytes:
    if size == 0:
        return destBuffer
    if isinstance(destBuffer, bytearray):
        destBuffer[destStart:destStart + size] = [value] * size
        return destBuffer
    return destBuffer[:destStart] + bytes([value] * size) + destBuffer[destStart + size:]


def hostToNetworkBytes(data: bytes):
    return data if sys.byteorder == "big" else data[::-1]


def networkToHostBytes(data: bytes):
    return data if sys.byteorder == "big" else data[::-1]


inf = (1 << 64) - 1


def __check_numeric_or_scalar_value(x):
    return (isinstance(x, Number) or (isinstance(x, np.ndarray) and getattr(x, "shape", None) == () and
                                      isinstance(np.expand_dims(x, -1)[0], Number)))


def equal(x, y, tol=1e-9):
    if (__check_numeric_or_scalar_value(x) and __check_numeric_or_scalar_value(y) and
            ((x == np.inf and y == np.inf) or (x == -np.inf and y == -np.inf))):
        return True
    return np.all(np.abs(x - y) < tol)


def greater(x, y, tol=1e-9):
    if (__check_numeric_or_scalar_value(x) and __check_numeric_or_scalar_value(y) and
            ((x == np.inf and y == np.inf) or (x == -np.inf and y == -np.inf))):
        return False
    return np.all((x - y) > tol)


def greater_equal(x, y, tol=1e-9):
    if (__check_numeric_or_scalar_value(x) and __check_numeric_or_scalar_value(y) and
            ((x == np.inf and y == np.inf) or (x == -np.inf and y == -np.inf))):
        return True
    return np.all((x - y) > -tol)


def less(x, y, tol=1e-9):
    if (__check_numeric_or_scalar_value(x) and __check_numeric_or_scalar_value(y) and
            ((x == np.inf and y == np.inf) or (x == -np.inf and y == -np.inf))):
        return False
    return np.all((x - y) < -tol)


def less_equal(x, y, tol=1e-9):
    if (__check_numeric_or_scalar_value(x) and __check_numeric_or_scalar_value(y) and
            ((x == np.inf and y == np.inf) or (x == -np.inf and y == -np.inf))):
        return True
    return np.all((x - y) < tol)


def rd(x, tol=1e-9):
    return 0 if equal(x, 0, tol) else x


def remove_duplicate_spaces_in_string(s):
    return re.sub(" +", " ", s)


first_cap_re = re.compile('(.)([A-Z][a-z]+)')
my_first_cap_re = re.compile('([A-Z]+|\S)([A-Z][a-z]+)')
all_cap_re = [re.compile('([a-z0-9])([A-Z])')]
my_all_cap_re = [re.compile('([0-9]+)([A-Za-z])'), re.compile('([A-Za-z])([0-9]+)'), re.compile('([a-z])([A-Z])')]


def convert_camel_case_to_snake_case(name: str, number_underscore: bool = True) -> str:
    """

    :param name: string to be renamed
    :param number_underscore: whether to put underscores in front of numbers or not
    :return:
    """
    _res = my_first_cap_re.sub(r'\1_\2', name) if number_underscore else first_cap_re.sub(r'\1_\2', name)
    for _re in (my_all_cap_re if number_underscore else all_cap_re):
        _res = _re.sub(r'\1_\2', _res)
    return re.sub("_+", "_", _res.lower())


def convert_snake_case_to_camel_case(name: str, cap_first: bool = False):
    components = name.split('_')
    # We capitalize the first letter of each component with the 'title' method and join them together.
    return (components[0].title() if cap_first else components[0]) + ''.join(x.title() for x in components[1:])


def read_file(filename: str) -> List[str]:
    with open(filename, "r") as f:
        content = [x.split("#")[0].strip() for x in f.readlines() if x.split("#")[0].strip()]
    return content


def float_exp_format(val, precision, exp_digits=-1):
    s = "%.*e" % (precision, val)
    if exp_digits > -1:
        mantissa, exp = s.split('e')
        # add 1 to digits as 1 is taken by sign +/-
        return "%se%+0*d" % (mantissa, exp_digits + 1, int(exp))
    else:
        return s


number_re = re.compile("[0-9]+")


def remove_number_from_name(name: str):
    return number_re.sub('', name)


def split_number_and_name_from_name(name: str, splitter: str = "_ :") -> Tuple:
    if len(splitter) == 0:
        return name,

    splitter_char = splitter[0]
    remaining_splitter = splitter[1:]
    splits = name.split(splitter_char)

    joined_name = []  # type: List
    __empty__ = "__empty__"
    text = __empty__
    for i, split_item in enumerate(splits):
        split_part = split_number_and_name_from_name(split_item, remaining_splitter)
        if len(split_part) > 1:
            start = 0
            end = len(split_part)

            to_append = ""
            if text != __empty__:
                to_append = text + splitter_char

            if not isinstance(split_part[0], int):
                to_append += split_part[0]
                start += 1
            joined_name.append(to_append)

            text = __empty__
            if not isinstance(split_part[-1], int):
                text = split_part[-1]
                end -= 1
                assert end >= 0

            joined_name = sum([joined_name, list(split_part[start:end])], [])
            continue

        split_content = split_part[0]
        is_number = False
        try:
            int(split_content)
            is_number = True
        except ValueError:
            pass
        if is_number:
            if text != __empty__:
                joined_name.append(text + splitter_char)
                text = __empty__
            joined_name.append(int(split_content))
        else:
            if text == __empty__:
                text = ""
            if i > 0:
                text += splitter_char
            text += split_content
    if text != __empty__:
        joined_name.append(text)
    return tuple(joined_name)
