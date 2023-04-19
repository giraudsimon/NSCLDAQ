import sys
from ctypes import create_string_buffer
from bitarray import bitarray, bits2bytes

"""
converters.py

Utilities for converting between data types.

Methods:
    str2char(): Convert a Python string to C-type char*.
    ba2int(): Convert a bitarray to an integer.
    int2ba(): Convert an integer to a bitarray.
"""

_is_py2 = bool(sys.version_info[0] == 2)

def str2char(pystr):
    """
    Convert Python string to char*.

    Arguments:
        pystr (str): Python string.
    
    Returns:
        char*: Mutable memory string buffer generated from input Python string.
    """    
    bstr = pystr.encode("utf-8")
    return create_string_buffer(bstr)

def ba2int(a, signed=False):
    """
    Convert the given bitarray to an integer. The bit-endianness of the 
    bitarray is respected. `signed` indicates whether two's complement is used 
    to represent the integer. Added for compatibility with older versions of 
    bitarray found in the Debian repos that do not install with the utilities 
    module. 

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Copied from tag 1.6.3, which is the version of the bitarray module 
    available in the Debian 11 repo.

    Arguments:
        a (bitarray): bitarray to convert.
        signed (bool): Two's complement.

    Raises:
        TypeError: If the argument to convert is not a bitarray.
        ValueError: If the bitarray is empty (length 0).

    Returns:
        int: Integer representation of the bitarray.
    """
    if not isinstance(a, bitarray):
        raise TypeError("bitarray expected")
    length = len(a)
    if length == 0:
        raise ValueError("non-empty bitarray expected")

    big_endian = bool(a.endian() == 'big')
    # for big endian pad leading zeros - for little endian we don't need to
    # pad trailing zeros, as .tobytes() will treat them as zero
    if big_endian and length % 8:
        a = bitarray(8 - length % 8, 'big') + a
        a.setall(0)
    b = a.tobytes()

    if _is_py2:
        c = bytearray(b)
        res = 0
        j = len(c) - 1 if big_endian else 0
        for x in c:
            res |= x << 8 * j
            j += -1 if big_endian else 1
    else: # py3
        res = int.from_bytes(b, byteorder=a.endian())

    if signed and res >= 1 << (length - 1):
        res -= 1 << length
    return res

def int2ba(i, length=None, endian=None, signed=False):
    """
    Convert the given integer to a bitarray (with given endianness, and no 
    leading (big-endian) / trailing (little-endian) zeros), unless the `length`
    of the bitarray is provided.  An `OverflowError` is raised if the integer
    is not representable with the given number of bits. `signed` determines 
    whether two's complement is used to represent the integer, and requires 
    `length` to be provided. If signed is False and a negative integer is
    given, an OverflowError is raised.

    See https://github.com/ilanschnell/bitarray/blob/master/bitarray/util.py.
    Copied from tag 1.6.3, which is the version of the bitarray module 
    available in the Debian 11 repo.
    
    Arguments:
        i (int): Integer to convert to bitarray.
        length (int): Length of the bitarray (optional parameter, 
                      default = None).
        endian (str): Endianness of the bitarray (optional parameter, 
                      default = None).
        signed (bool): Two's complement signed (optional parameter, 
                       default = False)
    Raises:
        TypeError: If the argument to convert is not an integer.
        TypeError: If no output bitarray length is provided.
        ValueError: The output bitarray length is zero.
        TypeError: Output is a signed integer and length is zero.
        OverflowError: If the integer value is too large to be represented by 
                       the number of provided bits.

    Returns:
        int: bitarray representation of the integer.
    """
    if not isinstance(i, (int, long) if _is_py2 else int):
        raise TypeError("integer expected")
    if length is not None:
        if not isinstance(length, int):
            raise TypeError("integer expected for length")
        if length <= 0:
            raise ValueError("integer larger than 0 expected for length")
    if signed and length is None:
        raise TypeError("signed requires length")

    if i == 0:
        # there are special cases for 0 which we'd rather not deal with below
        za = bitarray(length or 1, endian)
        za.setall(0)
        return za

    if signed:
        if i >= 1 << (length - 1) or i < -(1 << (length - 1)):
            raise OverflowError("signed integer out of range")
        if i < 0:
            i += 1 << length
    elif i < 0 or (length and i >= 1 << length):
        raise OverflowError("unsigned integer out of range")

    a = bitarray(0, endian or get_default_endian())
    big_endian = bool(a.endian() == 'big')
    if _is_py2:
        c = bytearray()
        while i:
            i, r = divmod(i, 256)
            c.append(r)
        if big_endian:
            c.reverse()
        b = bytes(c)
    else: # py3
        b = i.to_bytes(bits2bytes(i.bit_length()), byteorder=a.endian())

    a.frombytes(b)
    if length is None:
        return strip(a, 'left' if big_endian else 'right')

    la = len(a)
    if la > length:
        a = a[-length:] if big_endian else a[:length]
    if la < length:
        pad = bitarray(length - la, endian)
        pad.setall(0)
        a = pad + a if big_endian else a + pad
    assert len(a) == length
    return a
