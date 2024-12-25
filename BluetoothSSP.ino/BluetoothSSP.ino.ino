#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLESecurity.h>
#include <BLEAdvertisedDevice.h>

// Define the BLE service and characteristic UUIDs
#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID    "87654321-4321-8765-4321-abcdef123456"

// Initialize BLE server and characteristic
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

// Server callback for BLE connection events
class MyServerCallbacks : public BLEServerCallbacks
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
};

// Security callback for pairing and encryption events
class MySecurityCallbacks : public BLESecurityCallbacks
{
  bool onSecurityRequest() override 
  {
    Serial.println("onSecurityRequest");
    return true;
  }

  uint32_t onPassKeyRequest() override
  {
    Serial.println("onPassKeyRequest");
    Serial.println("Enter the remote passkey:");

    String input = "";
    unsigned long startTime = millis();  // Record the start time
    unsigned long timeout = 25000;       // 30-second timeout

    // Wait until a valid passkey is entered
    while (input.length() == 0)
    {
        if (Serial.available() > 0)
        {
            input = Serial.readStringUntil('\n');  // Read input until newline
            input.trim();  // Remove any leading/trailing spaces/newlines
        }
        
        if (millis() - startTime > timeout)
        {  // Check if 30 seconds have passed
            Serial.println("Timeout occurred, using default passkey.");
            input = String("123456");  // Default passkey after timeout
            break;  // Exit loop
        }

        delay(10);  // Give time for the serial input to be available
    }

    uint32_t passkeyEntered = input.toInt(); // Convert input to integer
    input = "";

    // Ensure the passkey is a valid 6-digit number
    if (passkeyEntered < 000000 || passkeyEntered > 999999)
    {
        Serial.println("Invalid passkey entered. Using 123456");
        passkeyEntered = 123456;  // Default passkey if input is invalid
    }

    Serial.print("Passkey entered: ");
    Serial.println(passkeyEntered);

    return passkeyEntered;  // Return the passkey to be used for pairing
  }

  void onPassKeyNotify(uint32_t passkey) override
  {
    Serial.println("onPasskeyNotify");
    Serial.print("Passkey to enter on other device: ");
    Serial.println(passkey);
  }

  bool onConfirmPIN(uint32_t pin) override
  {
    Serial.println("onConfirmPIN");
    Serial.print("Accept PIN? 'y': ");
    Serial.println(pin);
    
    String input = "";
    unsigned long startTime = millis();  // Record the start time
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

    // Check if the user confirmed the PIN
    if (input.equals("y") || input.equals("Y"))
    {
        Serial.println("PIN accepted");
        return true;
    }
    else
    {
        Serial.println("PIN rejected");
        return false;
    }
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
  }
};

// Setup BLE server and security settings
void setup()
{
  Serial.begin(115200);
  BLEDevice::init("ESP32_SecureBLE");  // Initialize the BLE device

  pServer = BLEDevice::createServer();  // Create the BLE server
  
  // Set the server callbacks for connection/disconnection
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE service and characteristic
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue("Hello World");
  pService->start();  // Start the service

  // Set up security
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND); // Enable bonding
  pSecurity->setStaticPIN(654321);

  // A >> confirm on other device only
  // W + Consent >> | confirm on other device only
  // W + DisplayPin >> RequiredHandlerNotRegistered
  // W + ProvidePin >> RequiredHandlerNotRegistered
  // W + ConfirmPinMatch >> RequiredHandlerNotRegistered
  // W + ProvidePasswordCredential >> RequiredHandlerNotRegistered | Apparently not supported in 4.1
  // pSecurity->setCapability(ESP_IO_CAP_NONE);

  // A >> onPasskeyNotify | PIN displayed.
  // W + Consent >> | confirm on other device only
  // W + DisplayPin >> RequiredHandlerNotRegistered | Apparently this device must ENTER the "Displayed PIn" from the other device, which OUT cant do
  // W + ProvidePin >> onPasskeyNotify >> PIN Displayed. | Apparently this device must show the "Provided Pin" which OUT can do
  // W + ConfirmPinMatch >> RequiredHandlerNotRegistered
  // W + ProvidePasswordCredential >> RequiredHandlerNotRegistered | Apparently not supported in 4.1
  // W + Any >> Provide Pin
  //  Has a display but no means to enter data. (screen-only). To display pairing information the user enters on the initiating device.
  // pSecurity->setCapability(ESP_IO_CAP_OUT);
  
  // A >> onPasskeyRequest | Remote PIN entered.
  // W + Consent >> | confirm on other device only
  // W + DisplayPin >> onPassKeyRequest >> Remote PIN entered. | Apparently this device must ENTER the "Displayed Pin" from the other device, which IN can do
  // W + ProvidePin >> onPassKeyRequest >> ??? This must be a bug
  // W + ProvidePin + StaticPIN >> onPassKeyRequest | static PIN Entered on both devices.
  // W + ConfirmPinMatch >> RequiredHandlerNotRegistered 
  // W + ConfirmPinMatch + StaticPIN >> onPassKeyRequest | A random PIN is Displayed on other device. Device ENTERs it , but also confirmed on other device which is useless,
  // W + ProvidePasswordCredential >> RequiredHandlerNotRegistered | Apparently not supported in 4.1
  // W + Any >> DisplayPin
  //  Has only a keyboard for input. Can type in a PIN provided by the initiating device.
  // pSecurity->setCapability(ESP_IO_CAP_IN);
  
  // A >> onPasskeyNotify | PIN displayed.
  // A + StaticPIN >> onConfirmPIN 
  // W + Consent >> | confirm on other device only | Each device asks for confirmation.
  // W + DisplayPin >> RequiredHandlerNotRegistered | Device ENTERs the "Displayed PIn" from the other device, which IO cant do
  // W + ProvidePin >> onPasskeyNotify >> PIN Displayed | Device SHOW the "Provided Pin" which IO can do
  // W + ConfirmPinMatch >> RequiredHandlerNotRegistered | Bug?
  // W + ConfirmPinMatch + StaticPIN >> onConfirmPIN | On each device a matching random PIN spawns and asks for confirmation.
  // W + ProvidePasswordCredential >> RequiredHandlerNotRegistered | Apparently not supported in 4.1
  // W + Any >> ProvidePin
  // W + Any + StaticPin >> ConfirmPinMatch
  //  Has both a display which can show a PIN, and input capability for "Yes" or "No" confirmation only. Secure pairing with mutual PIN or key confirmation.
  // pSecurity->setCapability(ESP_IO_CAP_IO);
  
  // A >> onPasskeyRequest | Remote PIN entered.
  // A + StaticPIN >> onConfirmPIN
  // W + Consent >> | confirm on other device only
  // W + DisplayPin >> onPassKeyRequest >> Remote PIN entered | Device ENTERs "Displayed PIn" from the other device, which KBDISP can do
  // W + ProvidePin >> onPasskeyNotify >> PIN Displayed | Device must SHOW the "Provided Pin" which KBDISP can do
  // W + ConfirmPinMatch >> onPassKeyRequest >> RequiredHandlerNotRegistered | Bug?
  // W + ConfirmPinMatch + StaticPIN >> onConfirmPIN | On each device a matching random PIN spawns and ask for confirmation.
  // W + ProvidePasswordCredential >> RequiredHandlerNotRegistered >> ESP_AUTH_SMP_NUM_COMP_FAIL | Apparently not supported in 4.1
  // W + Any >> DisplayPin
  // W + Any + StaticPin >> ConfirmPinMatch
  //  Has both a keyboard and a display. Allows advanced pairing options, such as entering and showing a PIN for verification.
  pSecurity->setCapability(ESP_IO_CAP_KBDISP);
  
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  
  BLEDevice::setSecurityCallbacks(new MySecurityCallbacks());

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("BLE Device is ready and advertising!");
}

void loop()
{
  delay(1000);  // Placeholder for BLE event handling
}
