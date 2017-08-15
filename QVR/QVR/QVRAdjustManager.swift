//
//  QVRAdjustViewController.swift
//  QVR
//
//  Created by 谌启亮 on 2017/5/17.
//  Copyright © 2017年 Tencent. All rights reserved.
//

import UIKit
import CoreBluetooth

let QVRServiceUUID: String = "F5EEC0AE-25F0-495F-999B-E7F36C9CB9A8"
let QVRCharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27FB"

let QVRViewAngleCharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27F1"
let QVRK1CharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27F2"
let QVRK2CharacterUUID: String = "99108141-4E4F-43B0-B4B6-DDD6713B27F3"

@objc public protocol QVRAdjustManagerDelegate: NSObjectProtocol {
    func viewAngleChange(_ viewAngle:String)
    func k1Change(_ k1:String)
    func k2Change(_ k2:String)
}


class QVRAdjustManager: NSObject, CBPeripheralManagerDelegate {
    
    
    
    var peripheralManager: CBPeripheralManager?
    public weak var delegate: QVRAdjustManagerDelegate?
    
    override init() {
        super.init()
        peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
    }
    
    public func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
        if #available(iOS 10.0, *) {
            if peripheral.state == CBManagerState.poweredOn {
                print("CBPeripheralManager power on")
                let characteristic = CBMutableCharacteristic(type: CBUUID(string: QVRCharacterUUID), properties: [.write, .read], value: nil, permissions: [.writeable, .readable])
                let service = CBMutableService(type: CBUUID(string: QVRServiceUUID), primary: true);
                
                let characteristicViewAngle = CBMutableCharacteristic(type: CBUUID(string: QVRViewAngleCharacterUUID), properties: [.write, .read], value: nil, permissions: [.writeable, .readable])
                let characteristicK1 = CBMutableCharacteristic(type: CBUUID(string: QVRK1CharacterUUID), properties: [.write, .read], value: nil, permissions: [.writeable, .readable])
                let characteristicK2 = CBMutableCharacteristic(type: CBUUID(string: QVRK2CharacterUUID), properties: [.write, .read], value: nil, permissions: [.writeable, .readable])

                service.characteristics = [characteristic, characteristicViewAngle, characteristicK1, characteristicK2];
                
                peripheral.add(service)
            }
        }
    }
    
    public func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
        print("CBPeripheralManager didAdd service")
        peripheral.startAdvertising([CBAdvertisementDataLocalNameKey:"Haha",CBAdvertisementDataServiceUUIDsKey:[CBUUID(string:QVRServiceUUID)]])
    }
    
    public func peripheralManagerDidStartAdvertising(_ peripheral: CBPeripheralManager, error: Error?) {
        print("peripheralManagerDidStartAdvertising")
    }
    
    public func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveWrite requests: [CBATTRequest]) {
        print("didReceiveWrite")
        for request in requests {
            peripheral.respond(to: request, withResult: .success)
            if let value = request.value {
                if var string = String(data: value, encoding: .utf8) {
                    if string.hasPrefix("{") && string.hasSuffix("}") {
                        string = string.replacingOccurrences(of: "{", with: "")
                        string = string.replacingOccurrences(of: "}", with: "")
                        switch request.characteristic.uuid.uuidString {
                        case QVRViewAngleCharacterUUID:
                            delegate?.viewAngleChange(string);
                        case QVRK1CharacterUUID:
                            delegate?.k1Change(string);
                        case QVRK2CharacterUUID:
                            delegate?.k2Change(string);
                        default:
                            break
                        }
                    }
                }
            }
            
        }
    }
    
    public func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didUnsubscribeFrom characteristic: CBCharacteristic){
        
    }

}
