#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLESecurity.h>
#include <BLEAdvertisedDevice.h>

// Define the BLE service and characteristic UUIDs
#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID    "87654321-4321-8765-4321-abcdef123456"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLESecurity* pSecurity = new BLESecurity();

byte inputMode; 
uint8_t encryptionMask = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK | ESP_BLE_CSR_KEY_MASK;

// Server callback for BLE connection events
class ServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer)
  {
    Serial.println("onConnect");
  }

  void onDisconnect(BLEServer* pServer)
  {
    Serial.println("onDisconnect, restarting advertising...");
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();
  }

  void onMtuChanged(BLEServer* pServer)
  {
     Serial.println("onMtuChanged");
  }
};

// Security callback for pairing and encryption events
class SecurityCallbacks : public BLESecurityCallbacks
{
  bool onSecurityRequest() override 
  {
    Serial.println("onSecurityRequest");
    inputMode = 1;
    return true;
  }

  void onPassKeyNotify(uint32_t passkey) override
  {
    Serial.println("onPasskeyNotify");
    Serial.print("Passkey to enter on other device: ");
    Serial.println(passkey);
  }
  
  uint32_t onPassKeyRequest() override
  {
    Serial.println("onPassKeyRequest");
    Serial.println("Enter the remote passkey:");

    String input = "";
    unsigned long startTime = millis();
    unsigned long timeout = 25000;

    // Wait until a valid passkey is entered
    while (input.length() == 0)
    {
        if (Serial.available() > 0)
        {
            input = Serial.readStringUntil('\n');  // Read input until newline
            input.trim();  // Remove any leading/trailing spaces/newlines
        }
        
        if (millis() - startTime > timeout)
        {
            Serial.println("Timeout occurred, using default passkey.");
            input = String("000000");  // Default passkey after timeout
            break;
        }

        delay(10); // Give time for the serial input to be available
    }

    uint32_t passkeyEntered = input.toInt(); // Convert input to integer
    // Ensure the passkey is a valid 6-digit number
    if (passkeyEntered < 000000 || passkeyEntered > 999999)
        Serial.println("Invalid passkey entered");
    else
    {
      Serial.print("Passkey entered: ");
      Serial.println(passkeyEntered);
    }

    String reamins = Serial.readString();
    Serial.println("left in buffer");
    Serial.println(reamins);
    return passkeyEntered;
  }

  bool onConfirmPIN(uint32_t pin) override
  {
    Serial.println("onConfirmPIN");
    Serial.print("Accept PIN? 'y': ");
    Serial.println(pin);
    
    String input = "";
    unsigned long startTime = millis();
    unsigned long timeout = 25000;

    // Wait until a valid passkey is entered
    while (input.length() == 0)
    {
        if (Serial.available() > 0)
        {
          input = Serial.readStringUntil('\n');  // Read input until newline
          input.trim();  // Remove any leading/trailing spaces/newlines
        }

        if (millis() - startTime > timeout)  // Check if timeout have passed
        {
          Serial.println("Timeout occurred, using default passkey.");
          input = "n";
          break;
        }

        delay(10);  // Give time for the serial input to be available
    }

    bool entry;
    if (input.equals("y") || input.equals("Y"))
    {
      entry = true;
      Serial.println("PIN accepted");
    }
    else
    {
      entry = false;
      Serial.println("PIN rejected");
    }

    String reamins = Serial.readString();
    Serial.println("left in buffer");
    Serial.println(reamins);
    return entry;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) override
  {
    Serial.println("onAuthenticationComplete");

    if (cmpl.success)
      Serial.println("Authentication Success!");
    else
    {
      Serial.println("Authentication Failed!");
      switch (cmpl.fail_reason)
      {
        case ESP_AUTH_SMP_PASSKEY_FAIL: Serial.println("Passkey failed"); break;
        case ESP_AUTH_SMP_OOB_FAIL: Serial.println("OOB data not available"); break;
        case ESP_AUTH_SMP_PAIR_AUTH_FAIL: Serial.println("Authentication requirements cannot be met"); break;
        case ESP_AUTH_SMP_CONFIRM_VALUE_FAIL: Serial.println("Confirm value does not match"); break;
        case ESP_AUTH_SMP_PAIR_NOT_SUPPORT: Serial.println("Pairing not supported"); break;
        case ESP_AUTH_SMP_ENC_KEY_SIZE: Serial.println("Encryption key size too short"); break;
        case ESP_AUTH_SMP_INVALID_CMD: Serial.println("Invalid SMP command"); break;
        case ESP_AUTH_SMP_UNKNOWN_ERR: Serial.println("Unknown pairing error"); break;
        case ESP_AUTH_SMP_REPEATED_ATTEMPT: Serial.println("Repeated pairing attempt"); break;
        case ESP_AUTH_SMP_INVALID_PARAMETERS: Serial.println("Invalid parameters"); break;
        case ESP_AUTH_SMP_DHKEY_CHK_FAIL: Serial.println("DHKey check failed"); break;
        case ESP_AUTH_SMP_NUM_COMP_FAIL: Serial.println("Numeric comparison failed"); break;
        case ESP_AUTH_SMP_BR_PARING_IN_PROGR: Serial.println("BR/EDR pairing in progress"); break;
        case ESP_AUTH_SMP_XTRANS_DERIVE_NOT_ALLOW: Serial.println("BR/EDR or BLE LTK cannot be used to derive"); break;
        case ESP_AUTH_SMP_INTERNAL_ERR: Serial.println("Internal pairing error"); break;
        case ESP_AUTH_SMP_UNKNOWN_IO: Serial.println("Unknown IO capability"); break;
        case ESP_AUTH_SMP_INIT_FAIL: Serial.println("Pairing initiation failed"); break;
        case ESP_AUTH_SMP_CONFIRM_FAIL: Serial.println("Confirm fail"); break;
        case ESP_AUTH_SMP_BUSY: Serial.println("Security request pending"); break;
        case ESP_AUTH_SMP_ENC_FAIL: Serial.println("Encryption failed"); break;
        case ESP_AUTH_SMP_STARTED: Serial.println("Pairing process started"); break;
        case ESP_AUTH_SMP_RSP_TIMEOUT: Serial.println("SMP response timeout"); break;
        case ESP_AUTH_SMP_DIV_NOT_AVAIL: Serial.println("Encrypted Diversifier value not available"); break;
        case ESP_AUTH_SMP_UNSPEC_ERR: Serial.println("Unspecified failure reason"); break;
        case ESP_AUTH_SMP_CONN_TOUT: Serial.println("Pairing connection timeout"); break;
        default: Serial.println("Unknown fail reason:");
      }
    }  
  
    inputMode = 0;
  }
};

void setup()
{
  Serial.begin(115200);
  BLEDevice::init("ESP32 LE SC");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue("Hello World");
  pService->start();

  // to work with Windows 11 static pin must be set before other security parameters
  // pSecurity->setStaticPIN(654321); // only works when negotiated mode is peripheral out initiator in 
  pSecurity->setCapability(ESP_IO_CAP_KBDISP);
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setInitEncryptionKey(encryptionMask);
  pSecurity->setRespEncryptionKey(encryptionMask);

  BLEDevice::setSecurityCallbacks(new SecurityCallbacks());

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("Device started advertising!");
  Serial.println("");
}

void loop()
{
  while (inputMode != 0)
  {
    delay(10);
  }

  String input = "";

  Serial.println("");
  Serial.println("Set Capabilities:");
  Serial.println("- No input or output '`0");
  Serial.println("- Display only '1'");
  Serial.println("- Keyboard only '2'");
  Serial.println("- Display and Confirm '3'");
  Serial.println("- Keyboard and Display '4'");
  Serial.println("");
  Serial.println("Set Authorization:");
  Serial.println("- No Bonding 'N'");
  Serial.println("- Only Bonding 'B'");
  Serial.println("- Only MITM 'M'");
  Serial.println("- Only Secure Connections 'S'");
  Serial.println("- Secure Connections + Bonding '7'");
  Serial.println("- Secure Connections + MITM '8'");
  Serial.println("- Secure Connections with MITM and Bonding '9'");
  Serial.println("");
  Serial.println("Set Encryption");
  Serial.println("- Toggle LTK Encryption key exchange 'X'");
  Serial.println("- Toggle IRK Encryption key exchange 'Y'");
  Serial.println("- Toggle CSRK Encryption key exchange 'Z'");
  Serial.println("");
  Serial.println("Set Static PIN 'P'");
  Serial.println("");

  // Wait until a valid passkey is entered
  while (input.length() == 0)
  {
      if (Serial.available() > 0)
      {
        input = Serial.readStringUntil('\n');  // Read input until newline
        input.trim();  // Remove any leading/trailing chars
      }

      delay(20);  // Give time for the serial input to be available
  }

  if (input.equals("0"))
  {
    pSecurity->setCapability(ESP_IO_CAP_NONE);
    Serial.println("-> No input or output");
  }
  else if (input.equals("1"))
  {
    pSecurity->setCapability(ESP_IO_CAP_OUT);
    Serial.println("-> Display only");
  }
  else if (input.equals("2"))
  {
    pSecurity->setCapability(ESP_IO_CAP_IN);
    Serial.println("-> Keyboard only");
  }
  else if (input.equals("3"))
  {
    pSecurity->setCapability(ESP_IO_CAP_IO);
    Serial.println("-> Display and Confirm");
  }
  else if (input.equals("4"))
  {
    pSecurity->setCapability(ESP_IO_CAP_KBDISP);
    Serial.println("-> Keyboard and Display");
  }
  else if (input.equals("N") || input.equals("n"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND);
    Serial.println("-> No Bonding");
  }
  else if (input.equals("B") || input.equals("b"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
    Serial.println("Only Bonding");
  }
  else if (input.equals("M") || input.equals("m"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_MITM);
    Serial.println("-> Only MITM");
  }
  else if (input.equals("S") || input.equals("s"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
    Serial.println("-> Only Secure Connections");
  }
  else if (input.equals("7"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    Serial.println("-> Secure Connections + Bonding");
  }
  else if (input.equals("8"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM);
    Serial.println("-> Secure Connections + MITM");
  }
  else if (input.equals("9"))
  {
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
    Serial.println("-> Secure Connections with MITM and Bonding");
  }
  else if (input.equals("X") || input.equals("x"))
  {
    encryptionMask ^= ESP_BLE_ENC_KEY_MASK;
    pSecurity->setInitEncryptionKey(encryptionMask);
    pSecurity->setRespEncryptionKey(encryptionMask);
    Serial.print("-> Toggled LTK Encryption keys:");
    Serial.println(encryptionMask);
  }
  else if (input.equals("Y") || input.equals("y"))
  {
    encryptionMask ^= ESP_BLE_ID_KEY_MASK;
    pSecurity->setInitEncryptionKey(encryptionMask);
    pSecurity->setRespEncryptionKey(encryptionMask);
    Serial.print("-> Toggled IRK Encryption keys:");
    Serial.println(encryptionMask);
  }
  else if (input.equals("Z") || input.equals("z"))
  {
    encryptionMask ^= ESP_BLE_CSR_KEY_MASK;
    pSecurity->setInitEncryptionKey(encryptionMask);
    pSecurity->setRespEncryptionKey(encryptionMask);
    Serial.print("-> Toggled CSRK Encryption keys:");
    Serial.println(encryptionMask);
  }
  else if (input.equals("P") || input.equals("p"))
  {
    // ToDo save and reapply pSecurity
    pSecurity->setStaticPIN(654321); // only works when negotiated mode is peripheral out initiator in 
    pSecurity->setCapability(ESP_IO_CAP_KBDISP);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
    pSecurity->setInitEncryptionKey(encryptionMask);
    pSecurity->setRespEncryptionKey(encryptionMask);
    Serial.println("-> Static PIN");
  }

  input = "";
  Serial.println("");
}
