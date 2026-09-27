## MAC Address Essentials

- **Definition:** A **Media Access Control (MAC) address** is a unique **48-bit (6-byte)** physical hardware identifier assigned to a network interface controller (NIC) at manufacturing.

- **OSI Layer:** Operates at **Layer 2 (Data Link Layer)**, handling device-to-device communication within the local network segment.

- **Format:** Represented as 6 pairs of hexadecimal digits separated by colons or hyphens (e.g., `02:AB:CD:12:34:56`).

- **Structure:**
  
  - **First 3 Bytes (OUI):** Organizationally Unique Identifier assigned by the IEEE to the hardware manufacturer.
  
  - **Last 3 Bytes:** Unique device identifier assigned by the manufacturer.

## Why a MAC Address is Needed

- **Local Delivery:** IP addresses route traffic across networks (Layer 3), while MAC addresses deliver data packets to the specific physical hardware on the local subnet (Layer 2).

- **Address Resolution Protocol (ARP):** Translates logical IP addresses into physical MAC addresses for local data frames.

- **Network Access Control:** Enables access filtering on Wi-Fi routers and switches (MAC Filtering).

- **Direct Peer-to-Peer Links:** Powers connectionless, low-overhead protocols like **ESP-NOW** without requiring an IP assignment or router handshake.

## How to Change (Override) a MAC Address

- **Hardware vs. Software:**
  
  - **Hardware MAC:** Permanently burned into non-volatile memory (e.g., eFuse). **Cannot be rewritten.**
  
  - **Software Spoofing:** Overrides the hardware value in volatile memory during runtime. **Temporary**—reverts upon reboot unless executed in boot code.

- **How It Works in Code (ESP32 Example):**
  
  1. Initialize the network interface mode (`WiFi.mode(WIFI_STA);`).
  
  2. Call the driver system API (`esp_wifi_set_mac(WIFI_IF_STA, customMAC);`).
  
  3. Run the override function **before** initiating communications (such as `esp_now_init()`).

## Rules to Follow When Setting a Custom MAC Address

To ensure the network controller and drivers accept a custom MAC address, the **first byte** must satisfy three specific bit-level criteria:

- **Rule 1: Unicast Transmission (Bit 0 = 0)**
  
  - The lowest bit (Bit 0) of the first byte specifies target delivery type.
  
  - Must be `0` for an individual device (Unicast). A value of `1` designates Multicast/Broadcast.
  
  - **Hex Check:** The second digit of the first byte must be an **even number** (`0`, `2`, `4`, `6`, `8`, `A`, `C`, `E`).

- **Rule 2: Locally Administered Flag (Bit 1 = 1)**
  
  - The second-lowest bit (Bit 1) specifies address authority.
  
  - Must be `1` to inform drivers and routers that the address is a custom software override rather than an IEEE factory-assigned vendor ID.
  
  - **Hex Check:** The second digit of the first byte **must be `2`, `6`, `A`, or `E`**.

- **Rule 3: Avoid Restricted Addresses**
  
  - Do not use `FF:FF:FF:FF:FF:FF` (Reserved for network-wide Broadcast).
  
  - Do not use `00:00:00:00:00:00` (Invalid null address).
  
  - Do not use `00:XX:XX:XX:XX:XX` (Fails Bit 1 check for software overrides).

## Valid vs. Invalid Software MAC Examples

| **Custom Address**  | **Status**  | **Primary Reason**                                       |
| ------------------- | ----------- | -------------------------------------------------------- |
| `02:00:00:00:00:01` | **VALID**   | Bit 0 = 0 (Unicast), Bit 1 = 1 (Locally Administered)    |
| `12:34:56:78:9A:BC` | **VALID**   | Ends in `2` $\rightarrow$ passes all driver checks       |
| `00:00:00:00:00:01` | **INVALID** | Bit 1 = 0 (Claims to be an official vendor ID; rejected) |
| `01:AA:BB:CC:DD:EE` | **INVALID** | Bit 0 = 1 (Multicast address; rejected for station mode) |
