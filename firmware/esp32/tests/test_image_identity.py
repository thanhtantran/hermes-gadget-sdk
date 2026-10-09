"""Regression tests for the ESP32 linker's placement of the OTA tag literal."""
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from package_release import PackageError, app_identity


def image(payload):
    header = bytearray(256)
    header[0] = 0xE9
    struct.pack_into("<I", header, 32, 0xABCD5432)
    header[48:53] = b"0.2.0"
    header[80:93] = b"hermes_gadget"
    return bytes(header) + payload


class ImageIdentityTests(unittest.TestCase):
    def test_bare_scanner_literal_before_board(self):
        self.assertEqual(
            app_identity(image(b"HGBOARD=\0BOOT\0HGBOARD=esp32-2432s028-cyd\0")),
            ("0.2.0", "esp32-2432s028-cyd"),
        )

    def test_board_before_scanner_literal(self):
        self.assertEqual(
            app_identity(image(b"HGBOARD=esp32s3-breadboard\0HGBOARD=\0")),
            ("0.2.0", "esp32s3-breadboard"),
        )

    def test_bare_literals_do_not_identify_a_board(self):
        with self.assertRaises(PackageError):
            app_identity(image(b"HGBOARD=\0HGBOARD=\0"))

    def test_missing_tag_is_rejected(self):
        with self.assertRaises(PackageError):
            app_identity(image(b"no board"))


if __name__ == "__main__":
    unittest.main()
