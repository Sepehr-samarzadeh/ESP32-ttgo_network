#include <Preferences.h>
#include <WiFi.h>

Preferences preferences;

// Default credentials - make sure these are defined at the global scope
const char* default_ssid = "tothelobbyyougo";
const char* default_pass = "fuzzyocean086";

// These will hold the credentials we read from Preferences
char storedSSID[32]; 
char storedPass[32];

// Flag to track if we're testing new credentials
bool testing_new_credentials = false;
unsigned long test_start_time = 0;

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
  unsigned long run_time = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID, storedPass);
  
  Serial.print("Connecting to ");
  Serial.print(storedSSID);
  Serial.print("...");
  
  while(WiFi.status() != WL_CONNECTED && millis() - run_time < 60000) {
    Serial.print('.');
    delay(1000);
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
  } else {
    try_to_reconnect();
  }
}

void try_to_reconnect() {
  Serial.println("\nAttempting to reconnect...");
  WiFi.begin(storedSSID, storedPass);

  unsigned long reconnectStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - reconnectStart < 30000) {
    Serial.print('.');
    delay(1000);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconnected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nReconnection failed! Will retry after 10 seconds!");
  }
}

void saveCredentials(const char* ssid, const char* pass) {
  preferences.begin("credentials", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
  Serial.println("WiFi credentials saved to flash memory");
  
  // Update the current credentials in memory
  strncpy(storedSSID, ssid, sizeof(storedSSID) - 1);
  strncpy(storedPass, pass, sizeof(storedPass) - 1);
  storedSSID[sizeof(storedSSID) - 1] = '\0'; // Ensure null termination
  storedPass[sizeof(storedPass) - 1] = '\0'; // Ensure null termination
}

void loadCredentials() {
  preferences.begin("credentials", true); // true = read-only mode
  
  // Get the values, providing defaults if nothing is found
  String ssid = preferences.getString("ssid", default_ssid);
  String pass = preferences.getString("pass", default_pass);
  
  // Copy to our char arrays with length limit to prevent buffer overflow
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
  // Save test credentials (modify these to something different than defaults)
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
  Serial.println("If you see 'TestNetwork' as the SSID after restart, memory persistence is working!");
  
  testing_new_credentials = true;
  test_start_time = millis();
}

void resetToDefaultCredentials() {
  Serial.println("\n-----------------------------------------");
  Serial.println("MEMORY TEST: RESETTING TO DEFAULT CREDENTIALS");
  Serial.println("-----------------------------------------\n");
  
  saveCredentials(default_ssid, default_pass);
  
  Serial.println("Default credentials restored. Please restart the device.");
  Serial.println("After restart, check if default credentials are loaded.");
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
    else if (command.startsWith("save:")) {
      // Format: save:new_ssid:new_password
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
      Serial.println("  test  - Save test credentials");
      Serial.println("  reset - Reset to default credentials");
      Serial.println("  show  - Show current credentials");
      Serial.println("  save:ssid:password - Save custom credentials");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n--- ESP32 WiFi Manager with Persistent Credentials ---");
  
  // Load credentials from flash memory
  loadCredentials();
  
  // If this is first run, save the default credentials
  if (strcmp(storedSSID, default_ssid) == 0 && strcmp(storedPass, default_pass) == 0) {
    Serial.println("First run detected or using default credentials");
  }
  
  // Connect using stored credentials
  connect_to_WiFi();
  
  Serial.println("\nMemory Test Commands:");
  Serial.println("  Type 'test' to save test credentials");
  Serial.println("  Type 'reset' to reset to default credentials");
  Serial.println("  Type 'show' to show current credentials");
  Serial.println("  Type 'save:ssid:password' to save custom credentials");
}

void loop() {
  // Process serial commands to test memory persistence
  processSerialCommands();
  
  // If we're testing new credentials, wait a bit before trying to reconnect
  if (testing_new_credentials) {
    if (millis() - test_start_time > 10000) {
      testing_new_credentials = false;
      Serial.println("\nTrying to connect with new credentials...");
      connect_to_WiFi();
    }
  }
  else if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection lost! Reconnecting...");
    try_to_reconnect();
  }
  
  // Check connection status periodically
  delay(5000);
}