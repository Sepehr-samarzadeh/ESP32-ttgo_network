#include <Preferences.h>
#include <WiFi.h>

Preferences preferences;

const char* default_ssid = "tothelobbyyougo";
const char* default_pass = "fuzzyocean086";

char storedSSID[32]; 
char storedPass[32];

bool testing_new_credentials = false;
unsigned long test_start_time = 0;
bool wifi_connected = false;
bool abort_connection = false;

void scan_WiFi() {
  Serial.print("Scanning...");
  int n = WiFi.scanNetworks();
  Serial.println("Done");
  if (n == 0) {
    Serial.println("No available WiFi networks");
  } else {
    Serial.print("Available networks: ");
    Serial.println(n);
    for(int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm)");
      delay(10);
    }
  }
  Serial.println("");
}

void connect_to_WiFi() {
  abort_connection = false;
  unsigned long run_time = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID, storedPass);
  
  Serial.print("Connecting to ");
  Serial.print(storedSSID);
  Serial.print("...");
  Serial.println("\n(Type 'abort' to cancel connection attempt)");
  
  while(WiFi.status() != WL_CONNECTED && millis() - run_time < 60000 && !abort_connection) {
    Serial.print('.');
    
    // Check for abort command
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input == "abort") {
        abort_connection = true;
        Serial.println("\nConnection attempt aborted by user!");
        WiFi.disconnect();
        break;
      }
    }
    
    delay(1000);
  }
  
  if (abort_connection) {
    wifi_connected = false;
    return;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnection successful!");
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    wifi_connected = true;
  } else {
    Serial.println("\nConnection failed!");
    wifi_connected = false;
    try_to_reconnect();
  }
}

void try_to_reconnect() {
  abort_connection = false;
  Serial.println("\nAttempting to reconnect...");
  Serial.println("(Type 'abort' to cancel reconnection attempt)");
  WiFi.begin(storedSSID, storedPass);

  unsigned long reconnectStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - reconnectStart < 30000 && !abort_connection) {
    Serial.print('.');
    
    // Check for abort command
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input == "abort") {
        abort_connection = true;
        Serial.println("\nReconnection attempt aborted by user!");
        WiFi.disconnect();
        break;
      }
    }
    
    delay(1000);
  }
  
  if (abort_connection) {
    wifi_connected = false;
    return;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconnected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    wifi_connected = true;
  } else {
    Serial.println("\nReconnection failed! Will retry after 10 seconds!");
    wifi_connected = false;
  }
}

void saveCredentials(const char* ssid, const char* pass) {
  preferences.begin("credentials", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
  Serial.println("WiFi credentials saved to flash memory");
  
  strncpy(storedSSID, ssid, sizeof(storedSSID) - 1);
  strncpy(storedPass, pass, sizeof(storedPass) - 1);
  storedSSID[sizeof(storedSSID) - 1] = '\0';
  storedPass[sizeof(storedPass) - 1] = '\0';
}

void loadCredentials() {
  preferences.begin("credentials", true);
  String ssid = preferences.getString("ssid", default_ssid);
  String pass = preferences.getString("pass", default_pass);
  ssid.toCharArray(storedSSID, sizeof(storedSSID));
  pass.toCharArray(storedPass, sizeof(storedPass));
  preferences.end();

  Serial.println("\n-----------------------------------------");
  Serial.println("MEMORY TEST: LOADED STORED CREDENTIALS");
  Serial.print("SSID: ");
  Serial.println(storedSSID);
  Serial.print("Password: ");
  Serial.println(storedPass);
  Serial.println("-----------------------------------------\n");
}

void testMemoryPersistence() {
  const char* test_ssid = "TestNetwork";
  const char* test_pass = "TestPassword123";

  Serial.println("\n-----------------------------------------");
  Serial.println("MEMORY TEST: SAVING NEW TEST CREDENTIALS");
  Serial.print("New SSID: ");
  Serial.println(test_ssid);
  Serial.print("New Password: ");
  Serial.println(test_pass);
  Serial.println("-----------------------------------------\n");

  saveCredentials(test_ssid, test_pass);

  Serial.println("Test credentials saved. Please restart the device.");
  Serial.println("After restart, check if these test credentials are loaded.");

  testing_new_credentials = true;
  test_start_time = millis();
}

void resetToDefaultCredentials() {
  Serial.println("\n-----------------------------------------");
  Serial.println("MEMORY TEST: RESETTING TO DEFAULT CREDENTIALS");
  Serial.println("-----------------------------------------\n");

  saveCredentials(default_ssid, default_pass);

  Serial.println("Default credentials restored. Please restart the device.");
}

void processSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "test") {
      testMemoryPersistence();
    } 
    else if (command == "reset") {
      resetToDefaultCredentials();
    }
    else if (command == "show") {
      Serial.println("\n-----------------------------------------");
      Serial.println("CURRENT CREDENTIALS IN USE:");
      Serial.print("SSID: ");
      Serial.println(storedSSID);
      Serial.print("Password: ");
      Serial.println(storedPass);
      Serial.println("-----------------------------------------\n");
    }
    else if (command == "status") {
      Serial.println("\n-----------------------------------------");
      Serial.println("CURRENT WIFI STATUS:");
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("CONNECTED!");
        Serial.print("Network: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
      } else {
        Serial.println("DISCONNECTED");
      }
      Serial.println("-----------------------------------------\n");
    }
    else if (command == "scan") {
      scan_WiFi();
    }
    else if (command == "connect") {
      connect_to_WiFi();
    }
    else if (command == "abort") {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Disconnecting from WiFi...");
        WiFi.disconnect();
        wifi_connected = false;
        Serial.println("WiFi disconnected.");
      } else {
        Serial.println("Not currently connected to WiFi. Aborting any pending connection attempts.");
        abort_connection = true;
      }
    }
    else if (command.startsWith("save:")) {
      int firstColon = command.indexOf(':');
      int secondColon = command.indexOf(':', firstColon + 1);
      if (secondColon > firstColon) {
        String new_ssid = command.substring(firstColon + 1, secondColon);
        String new_pass = command.substring(secondColon + 1);
        char ssid_buf[32];
        char pass_buf[32];
        new_ssid.toCharArray(ssid_buf, sizeof(ssid_buf));
        new_pass.toCharArray(pass_buf, sizeof(pass_buf));
        saveCredentials(ssid_buf, pass_buf);
        Serial.println("New credentials saved. Restart to test persistence.");
      } else {
        Serial.println("Invalid format. Use: save:new_ssid:new_password");
      }
    }
    else {
      Serial.println("Available commands:");
      Serial.println("  test   - Save test credentials");
      Serial.println("  reset  - Reset to default credentials");
      Serial.println("  show   - Show current credentials");
      Serial.println("  status - Show WiFi connection status");
      Serial.println("  scan   - Scan for available WiFi networks");
      Serial.println("  connect - Try connecting with current credentials");
      Serial.println("  abort  - Abort connection attempt or disconnect WiFi");
      Serial.println("  save:ssid:password - Save custom credentials");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n--- ESP32 WiFi Manager with Persistent Credentials ---");
  
  loadCredentials();

  if (strcmp(storedSSID, default_ssid) == 0 && strcmp(storedPass, default_pass) == 0) {
    Serial.println("First run detected or using default credentials");
  }
  
  connect_to_WiFi();

  Serial.println("\nCommands:");
  Serial.println("  Type 'test' to save test credentials");
  Serial.println("  Type 'reset' to reset to default credentials");
  Serial.println("  Type 'show' to show current credentials");
  Serial.println("  Type 'status' to show WiFi connection status");
  Serial.println("  Type 'scan' to scan for available WiFi networks");
  Serial.println("  Type 'connect' to try connecting with current credentials");
  Serial.println("  Type 'abort' to abort connection attempt or disconnect WiFi");
  Serial.println("  Type 'save:ssid:password' to save custom credentials");
}

void loop() {
  processSerialCommands();

  static bool lastConnectionState = wifi_connected;
  bool currentConnectionState = (WiFi.status() == WL_CONNECTED);

  if (lastConnectionState != currentConnectionState) {
    if (currentConnectionState) {
      Serial.println("WiFi connected");
    } else {
      Serial.println("WiFi disconnected");
    }
    lastConnectionState = currentConnectionState;
  }

  if (testing_new_credentials) {
    if (millis() - test_start_time > 10000) {
      testing_new_credentials = false;
      Serial.println("\nTrying to connect with new credentials...");
      connect_to_WiFi();
    }
  }
  else if (WiFi.status() != WL_CONNECTED && lastConnectionState) {
    Serial.println("Connection lost! Reconnecting...");
    try_to_reconnect();
  }

  delay(1000);  // Reduced delay for more responsive command processing
}