# Esp32-Pairing Peripheral

A Bluetooth peripheral for testing initiator pairing implementations.\
Supports the ability to change IO Capabilities, Authentication and Encryption parameters on the fly.

## Features

| IO Capabilities   |                                                                        | HotKey |
| ----------------- | ---------------------------------------------------------------------- | ------ |
| ESP_IO_CAP_NONE   | Peripheral can display a PIN, has no inputhas no IO                    | 0      |
| ESP_IO_CAP_OUT    | Peripheral can display a PIN, has no input                             | 1      |
| ESP_IO_CAP_IN     | Peripheral enter input a PIN, has no display                           | 2      |
| ESP_IO_CAP_IO     | Peripheral can display a PIN, and enter binary confirmation (Y or Y/N) | 3      |
| ESP_IO_CAP_KBDISP | Peripheral can display a PIN and enter a PIN                           | 4      |

| Authentication Modes         |                                              | HotKey |
| ---------------------------- | -------------------------------------------- | ------ |
| ESP_LE_AUTH_NO_BOND          | No bonding                                   | N      |
| ESP_LE_AUTH_BOND             | Bonding                                      | B      |
| ESP_LE_AUTH_REC_MITM         | Man In The Middle protection                 | M      |
| ESP_LE_AUTH_REC_SC_ONLY      | Secure Connections                           | S      |
| ESP_LE_AUTH_REC_SC_BOND      | Secure Connections, Bonding                  | 7      |
| ESP_LE_AUTH_REC_SC_MITM      | Secure Connections, MITM protection          | 8      |
| ESP_LE_AUTH_REC_SC_MITM_BOND | Secure Connections, MITM protection, Bonding | 9      |

| Encryption           |                                                                                                        | HotKey |
| -------------------- | ------------------------------------------------------------------------------------------------------ | ------ |
| ESP_BLE_ENC_KEY_MASK | Toggle exchanging the encryption key (Long Term Key, LTK) to secure data transmission                  | X      |
| ESP_BLE_ID_KEY_MASK  | Toggle exchanging the Identity Resolving Key (IRK) to enhance privacy by resolving private addresses   | Y      |
| ESP_BLE_CSR_KEY_MASK | Toggle exchanging the connection signature resolving key (CSRK) Ensure data integrity and authenticity | Z      |

| Static PIN |                  | HotKey |
| ---------- | ---------------- | ------ |
| Set        | Set a static PIN | P      |

### Results

![](https://holocron.so/uploads/97cfcc85-image.png)

[ps://onedrive.live.com/view.aspx?resid=A6B4ADD4E4C05CFA!382&id=documents](https://onedrive.live.com/view.aspx?resid=A6B4ADD4E4C05CFA!382&id=documents)

...