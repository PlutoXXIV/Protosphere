ESP-NOW CODE BREAKDOWN GUIDE
===================================================================

==========================================================

#### 

#### 1. SERIAL COMMUNICATION

```cpp
Serial.begin(115200);
```

• Purpose: Initializes serial communication between the ESP32 and 
  your computer over the USB cable.
• Baud Rate (115200): Specifies the speed in bits per second (bps). 
  At 115200, the ESP32 sends 115,200 bits of data to your Serial 
  Monitor every second.
• Requirement: The value in code MUST match the baud rate selected 
  in your IDE Serial Monitor to prevent garbled "garbage" output.

-------------------------------------------------------------------



#### 2. WI-FI STATION MODE

```cpp
WiFi.mode(WIFI_STA);
```

• Purpose: Sets the ESP32 internal 802.11 radio controller into 
  Station (STA) mode.
• Wi-Fi Modes:
    - WIFI_STA (Station): Operates like a client device.
    - WIFI_AP  (Access Point): Operates like a router broadcasting SSID.
    - WIFI_OFF : Powers down the Wi-Fi radio entirely.
• Role in ESP-NOW: ESP-NOW works on Layer 2 (Data Link) of the Wi-Fi 
  stack. Setting WIFI_STA powers up the internal radio hardware and 
  assigns its base MAC address without needing to connect to a router.

-------------------------------------------------------------------



#### 3. DRIVER INITIALIZATION

```cpp
if (esp_now_init() != ESP_OK) return;
```

• Purpose: Allocates system memory and loads the ESP-NOW protocol stack.
• Error Check: esp_now_init() returns ESP_OK (integer 0) on success. 
  If memory allocation fails or Wi-Fi isn't ready, it returns an error.
• Execution Guard: The "return;" statement halts execution inside 
  setup() if initialization fails, keeping the ESP32 from crashing.

-------------------------------------------------------------------



#### 4. PEER REGISTRATION AND MEMORY COPY

```cpp
esp_now_peer_info_t peerInfo = {};
memcpy(peerInfo.peer_addr, receiverAddress, 6);
esp_now_add_peer(&peerInfo);
```

• Built-in Struct (esp_now_peer_info_t):
  A pre-defined C structure in esp_now.h containing peer parameters:
    - peer_addr[6] : 6-byte array for the target MAC address.
    - channel      : Wi-Fi channel (0 to 14).
    - encrypt      : Encryption toggle flag (true/false).

• Line Breakdown:

1. Instantiation: esp_now_peer_info_t peerInfo = {}; creates the 
   peerInfo variable and zeroes out all its internal fields.
2. Member Access: peerInfo.peer_addr accesses the built-in 6-byte 
   array inside the struct.
3. Memory Copy (memcpy): C arrays cannot be assigned using '='. 
   memcpy(destination, source, size) copies 6 raw bytes from 
   receiverAddress into peerInfo.peer_addr.
4. Registration: esp_now_add_peer(&peerInfo) passes the pointer (&) 
   of the struct to add the receiver to the ESP-NOW lookup table.

-------------------------------------------------------------------



#### 5. TYPE CASTING IN DATA TRANSMISSION

```cpp
esp_now_send(receiverAddress, (uint8_t *)msg, sizeof(msg));
```

• Parameters:
    - receiverAddress : Target peer MAC address array.
    - (uint8_t *)msg  : Pointer cast to the message memory address.
    - sizeof(msg)     : Total byte length of the message array.

• Type Casting Mechanics:

- msg is declared as a text array: char msg[] = "hello IoT";
- The esp_now_send() function expects a generic byte pointer: 
  const uint8_t *
- The explicit cast (uint8_t *) tells the compiler to treat the 
  memory address of the char array as raw byte data so it can be sent wirelessly.
