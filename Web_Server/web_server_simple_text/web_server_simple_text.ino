#include <WiFi.h>        // Library used to create Wi-Fi on the ESP32
#include <WebServer.h>   // Library used to create the web server


// ============================================================
// 1. Wi-Fi SETTINGS
// ============================================================

// Name of the Wi-Fi network that the ESP32 will create
const char* ssid = "ESP32_AP";

// Password for the ESP32 Wi-Fi network
// The password must be at least 8 characters long
const char* password = "12345678";


// ============================================================
// 2. CREATE THE WEB SERVER
// ============================================================

// Create a web server object.
// "80" is the standard port used for HTTP.
WebServer server(80);


// ============================================================
// 3. WEBPAGE THAT WILL APPEAR ON THE PHONE
// ============================================================

// This is the HTML webpage that the ESP32 will send
// to the phone when the phone visits the ESP32's IP address.
//
// The webpage contains:
// - A text box where the user can type a message
// - A button to send that message to the ESP32
//
const char webpage[] = R"rawliteral(

<!DOCTYPE html>

<html>

<head>
    <title>ESP32 Message</title>
</head>

<body>

    <h2>Send Message to ESP32</h2>

    <!--
      The form sends the typed message to the ESP32.

      action="/message"
      -> Send the form data to the "/message" URL.

      method="POST"
      -> Use an HTTP POST request to send the data.
    -->
    <form action="/message" method="POST">

        <!--
          This is the text box.

          name="msg"
          -> The name "msg" is used by the ESP32
             to identify the message that was typed.
        -->
        <input type="text" name="msg" required>

        <!-- Button used to submit the message -->
        <input type="submit" value="Send Message">

    </form>

</body>

</html>

)rawliteral";


// ============================================================
// 4. FUNCTION TO HANDLE THE MAIN WEBPAGE
// ============================================================

// This function runs when the phone visits:
//
//     http://192.168.4.1/
//
// The ESP32 sends the HTML webpage to the phone.
void handleHome()
{
    // send() sends a response from the ESP32 to the phone.
    //
    // 200        -> HTTP status code meaning "OK"
    // "text/html"-> The response contains HTML
    // webpage    -> The actual webpage we want to send
    server.send(200, "text/html", webpage);
}


// ============================================================
// 5. FUNCTION TO HANDLE THE MESSAGE FROM THE PHONE
// ============================================================

// This function runs when the phone submits the form.
//
// The browser sends a POST request to:
//
//     /message
//
// along with the message typed into the text box.
void handleMessage()
{
    // --------------------------------------------------------
    // Get the message sent by the phone
    // --------------------------------------------------------
    
    // "msg" is the same name that we used in the HTML:
    //
    // <input type="text" name="msg">
    //
    // server.arg("msg") retrieves whatever the user typed.
    String message = server.arg("msg");


    // --------------------------------------------------------
    // Print the message to the Serial Monitor
    // --------------------------------------------------------

    Serial.print("Message from phone: ");
    Serial.println(message);


    // --------------------------------------------------------
    // Send a response back to the phone
    // --------------------------------------------------------

    // After receiving the message, send a simple webpage
    // back to the phone so that the user knows the message
    // was received.
    server.send(
        200,
        "text/html",
        "<h2>Message received by ESP32!</h2>"
        "<a href='/'>Send another message</a>"
    );
}


// ============================================================
// 6. SETUP FUNCTION
// ============================================================

// setup() runs only ONE TIME when the ESP32 starts.
void setup()
{
    // --------------------------------------------------------
    // Start Serial communication
    // --------------------------------------------------------

    // 115200 is the communication speed (baud rate).
    //
    // The Serial Monitor in Arduino IDE must also be set
    // to 115200 baud.
    Serial.begin(115200);


    // --------------------------------------------------------
    // Start the ESP32 as a Wi-Fi Access Point
    // --------------------------------------------------------

    // The ESP32 will create its own Wi-Fi network.
    //
    // The phone can connect directly to this network.
    WiFi.softAP(ssid, password);


    // --------------------------------------------------------
    // Print the ESP32's IP address
    // --------------------------------------------------------

    // When the ESP32 creates its own Wi-Fi network,
    // it normally gets the IP address:
    //
    //     192.168.4.1
    //
    // We print it so that we can see the address
    // in the Serial Monitor.
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.softAPIP());


    // --------------------------------------------------------
    // Define what happens when someone visits "/"
    // --------------------------------------------------------

    // When the phone opens:
    //
    //     http://192.168.4.1/
    //
    // the handleHome() function will run.
    server.on("/", handleHome);


    // --------------------------------------------------------
    // Define what happens when "/message" receives POST data
    // --------------------------------------------------------

    // The phone's form sends the message using POST.
    //
    // Therefore we tell the ESP32:
    //
    //     If a POST request arrives at /message,
    //     run handleMessage().
    //
    server.on("/message", HTTP_POST, handleMessage);


    // --------------------------------------------------------
    // Start the web server
    // --------------------------------------------------------

    server.begin();

    Serial.println("Web server started!");
    Serial.println("Connect your phone to ESP32_AP");
    Serial.println("Then open http://192.168.4.1");
}


// ============================================================
// 7. LOOP FUNCTION
// ============================================================

// loop() runs continuously after setup() finishes.
void loop()
{
    // Check whether a phone/computer has sent an HTTP request.
    //
    // If a request has arrived, WebServer will determine
    // which function should handle it.
    server.handleClient();
}
