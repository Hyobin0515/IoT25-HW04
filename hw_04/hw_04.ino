#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE 서버 이름 설정 (본인 이름으로 변경)
#define DEVICE_NAME "hyobinchoi_ESP32"

// UUID 정의 (표준 형식 사용)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;  // 데이터 전송 간격 (밀리초)

// 연결 상태 관리를 위한 콜백 클래스
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("클라이언트가 연결되었습니다!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("클라이언트 연결이 끊어졌습니다!");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("BLE 서버 시작...");

  // BLE 장치 초기화
  BLEDevice::init(DEVICE_NAME);

  // BLE 서버 생성
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // BLE 서비스 생성
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // BLE 특성(Characteristic) 생성
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // BLE 디스크립터 추가
  pCharacteristic->addDescriptor(new BLE2902());

  // 서비스 시작
  pService->start();

  // 어드버타이징 설정 및 시작
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // iPhone 연결 이슈 해결용 
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE 서버가 시작되었습니다. nRF Connect 앱으로 연결해보세요!");
  Serial.println("장치 이름: " + String(DEVICE_NAME));
}

void loop() {
  // 현재 시간 확인
  unsigned long currentMillis = millis();

  // 장치 연결 상태 확인 및 데이터 전송
  if (deviceConnected) {
    // 일정 간격으로 데이터 전송
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      
      // 테스트 데이터 생성 (시간 기반)
      String data = "ESP32 BLE 테스트: " + String(currentMillis / 1000) + "초";
      
      // Characteristic에 데이터 설정 및 알림
      pCharacteristic->setValue(data.c_str());
      pCharacteristic->notify();
      
      Serial.println("알림 전송: " + data);
    }
  }

  // 연결 상태 변경 처리
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // 연결 종료 후 대기
    pServer->startAdvertising(); // 어드버타이징 재시작
    Serial.println("어드버타이징 시작...");
    oldDeviceConnected = deviceConnected;
  }
  
  // 새 연결 상태 처리
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("새 연결이 설정되었습니다!");
  }
}