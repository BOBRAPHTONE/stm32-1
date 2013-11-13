#!/usr/bin/env python


#!/usr/bin/env python
"""Pure python library for calculating CRC16"""

##############################################################################
#
#    Copyright (C) Gennady Trafimenkov, 2011
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
##############################################################################

#
# by pycrc v0.8.1, http://www.tty1.net/pycrc/
# using the configuration:
#    Width        = 16
#    Poly         = 0x8005
#    XorIn        = 0xffff
#    ReflectIn    = True
#    XorOut       = 0x0000
#    ReflectOut   = True
#    Algorithm    = table-driven
#
# Command:
# ./pycrc-0.8.1/pycrc.py --model=crc-16-modbus --generate=table  --algorithm=table-driven
CRC16_ANSI_TABLE = [
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
    ]


Init file with 0xa5a5
    implies numbytes( data + checksum )  % 16 == 0
Read into halfword array from file....walk through array n bytes
at a time....


[data][checksum]  read this, checksum confirm, pass [data] to struct for unpack.
....yadda
[data][checksum]  read this, checksum confirm, pass [data] to struct for unpack.
[data][checksum][0xa5a5]  read this, checksum confirm, pass [data] to struct for unpack.
    done.

while !eod marker??  (0xa5a5)

# compare to crc_16_reflect.c
# crc is the initial value ... probably 0xffff
# table is the lookup table for CRC 16 ANSI reflected
# len is the length of the data.
def crc16_update(data, crc, table, datalen):
    """
        data    is an  array of bytes
        crc     is the crc initial value
        table   is the lookup table for CRC
        datalen is the length of the data.
   
    Return CRC  (aka: the remainder)
    """
    final_value = 0x0
    for i in range(0,datalen):
        tbl_idx = (crc ^ data[i]) & 0xff;
        crc     = (table[tbl_idx] ^ (crc >> 8) & 0xffff;

    return crc ^ final_value;


def crc_16_ansi(data, crc=0xffff, datalen):
    """Calculate CRC-CCITT (XModem) variant of CRC16.
    `data`      - data for calculating CRC, must be a string
    `crc`       - initial value
    Return calculated value of CRC
    """
    return crc16_update(data, crc, CRC16_ANSI_TABLE, datalen)


from array import array
# Edit:
from sys import byteorder as system_endian # thanks, Sven!
# Sigh...
from os import stat

bytecount is GENERIC_message(?) + checksum(2) + check_eod_byte(2)
eod_byte is 0xa5a5

def read_file(filename, endian):
    count = stat(filename).st_size / 2
    with file(filename, 'rb') as f:
        result = array('b')
        result.fromfile(f, bytecount)
        if endian != system_endian: result.byteswap()
        return result

