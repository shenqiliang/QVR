//
//  ViewController.swift
//  QVRRemote
//
//  Created by 谌启亮 on 2017/5/17.
//  Copyright © 2017年 Tencent. All rights reserved.
//

import Cocoa
import CoreBluetooth

let QVRServiceUUID: String = "F5EEC0AE-25F0-495F-999B-E7F36C9CB9A8"
//let QVRServiceUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27FA"
let QVRCharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27FB"

let QVRViewAngleCharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27F1"
let QVRK1CharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27F2"
let QVRK2CharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27F3"

class ViewController: NSViewController, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    var central: CBCentralManager?
    var peripheral: CBPeripheral?
    var service: CBService?
    var characteristicViewAngle: CBCharacteristic?
    var characteristicK1: CBCharacteristic?
    var characteristicK2: CBCharacteristic?
    
    @IBOutlet var viewAngleSlider: NSSlider?
    @IBOutlet var k1Slider: NSSlider?
    @IBOutlet var k2Slider: NSSlider?

    override func keyDown(with event: NSEvent) {
        if let key = event.characters {
            switch key {
            case "7":
                viewAngleSlider?.floatValue-=0.1
                viewAngleAction(sender: viewAngleSlider!)
            case "8":
                viewAngleSlider?.floatValue+=0.1
                viewAngleAction(sender: viewAngleSlider!)
            case "4":
                k1Slider?.floatValue-=0.01
                k1Action(sender: k1Slider!)
            case "5":
                k1Slider?.floatValue+=0.01
                k1Action(sender: k1Slider!)
            case "1":
                k2Slider?.floatValue-=0.01
                k2Action(sender: k2Slider!)
            case "2":
                k2Slider?.floatValue+=0.01
                k2Action(sender: k2Slider!
                
                )
            default:
                break
            }
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        
        central = CBCentralManager(delegate: self, queue: nil);

        // Do any additional setup after loading the view.
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
    
    public func centralManagerDidUpdateState(_ central: CBCentralManager){
        if central.state == CBCentralManagerState.poweredOn {
            print("central power on")
            let uuids = [CBUUID(string: QVRServiceUUID)]
            central.scanForPeripherals(withServices: uuids, options: nil)
        }
    }
    
    public func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if self.peripheral == nil {
            print("did find peripherals ", peripheral)
            central.stopScan()
            self.peripheral = peripheral;
            central.connect(peripheral, options: nil)
        }
    }
    
    public func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("didConnect peripherals ", peripheral)
        peripheral.delegate = self;
        peripheral.discoverServices([CBUUID(string:QVRServiceUUID)])
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?){
        print("didDiscoverServices", peripheral.services ?? " no service")
        service = peripheral.services?.first;
        if let ser = service {
            peripheral.discoverCharacteristics([CBUUID(string: QVRViewAngleCharacterUUID),CBUUID(string: QVRK1CharacterUUID),CBUUID(string: QVRK2CharacterUUID)], for: ser)
        }
    }

    public func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?){
        print("didDiscoverCharacteristicsFor", service.characteristics ?? "empty")
        for characteristic in service.characteristics! {
            switch characteristic.uuid.uuidString {
            case QVRViewAngleCharacterUUID:
                characteristicViewAngle = characteristic
            case QVRK1CharacterUUID:
                characteristicK1 = characteristic
            case QVRK2CharacterUUID:
                characteristicK2 = characteristic
            default:
                break;
            }
        }
    }
    
    @IBAction public func viewAngleAction(sender: NSSlider) {
        let value = "{\(sender.floatValue)}"
        peripheral?.writeValue(value.data(using: .utf8)!, for: characteristicViewAngle!, type: .withResponse)
    }
    
    @IBAction public func k1Action(sender: NSSlider) {
        let value = "{\(sender.floatValue)}"
        peripheral?.writeValue(value.data(using: .utf8)!, for: characteristicK1!, type: .withResponse)
    }
    
    @IBAction public func k2Action(sender: NSSlider) {
        let value = "{\(sender.floatValue)}"
        peripheral?.writeValue(value.data(using: .utf8)!, for: characteristicK2!, type: .withResponse)
    }

    public func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        print("didWriteValueFor")
    }
    
    public func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("didDisconnectPeripheral")
        self.peripheral = nil
        let uuids = [CBUUID(string: QVRServiceUUID)]
        central.scanForPeripherals(withServices: uuids, options: nil)
    }
}

