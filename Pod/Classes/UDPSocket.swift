// The MIT License (MIT)
//
// Copyright (c) 2015 Joakim Gyllström
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

import Foundation

public final class UDPSocket : Asyncable, Transmittable, Connectable {
    // MARK: Private data
    private let commonSocket: Socket
    private let addressInformation: UnsafeMutablePointer<addrinfo>
    private var connectionDescriptor: Int32?
    
    // MARK: Async vars
    public var dispatchQueue: dispatch_queue_t {
        get {
            return commonSocket.dispatchQueue
        }
    }
    
    public var callbackQueue: dispatch_queue_t {
        get {
            return commonSocket.callbackQueue
        }
    }
    
    // MARK: Connectable vars
    public var connected: Bool {
        get {
            return connectionDescriptor != nil
        }
    }
    
    // MARK: Init
    public required init(host: String, port: UInt) {
        commonSocket = Socket(host: host,
            port: port,
            callback: dispatch_get_main_queue(),
            dispatch: dispatch_queue_create("TCP:\(host):\(port)", nil)
        )
        
        addressInformation = swocket_addrinfo_udp(commonSocket.host, commonSocket.port)
    }
    
    deinit {
        freeaddrinfo(addressInformation)
    }
    
    // MARK: Connectable functions
    public func connect() throws {
        if connectionDescriptor != nil {
            return
            // TODO: Should this be an error?
        }
        
        let temp = swocket_socket_udp(addressInformation)
        if temp != -1 {
            connectionDescriptor = temp
        } else {
            throw SwocketError.FailedToConnect
        }
    }
    public func disconnect() throws {
        guard let connectionDescriptor = connectionDescriptor else {
            return
            // TODO: Error?
        }
        
        close(connectionDescriptor)
        self.connectionDescriptor = nil
    }
    
    // MARK: Transmittable
    public func sendData(data: NSData) throws {
        if connectionDescriptor == nil {
            try connect()
        }
        
        try sendAll(data, descriptor: connectionDescriptor!, destination: addressInformation, totalSent: 0, bytesLeft: data.length, chunkSize: 0)
    }

    public func recieveData() throws -> NSData {
        if connectionDescriptor == nil {
            try connect()
        }
        
        var zero: Int8 = 0
        let data = NSMutableData(bytes: &zero, length: commonSocket.maxRecieveSize)
        
        let numberOfBytes = swocket_recieve_udp(connectionDescriptor!, data.mutableBytes, data.length)
        if numberOfBytes == -1 {
            throw SwocketError.FailedToRecieve
        } else {
            return NSData(bytes: data.bytes, length: numberOfBytes)
        }
    }
    
    // MARK: Private
    private func sendAll(data: NSData, descriptor: Int32, destination: UnsafeMutablePointer<addrinfo>, totalSent: Int, bytesLeft: Int, chunkSize: Int) throws {
        if chunkSize == -1 {
            throw SwocketError.FailedToSend
        } else if bytesLeft == 0 {
            return
        } else {
            let chunkSize = swocket_send_udp(descriptor, data.bytes+totalSent, bytesLeft, destination)
            try sendAll(data, descriptor: descriptor, destination: destination, totalSent: totalSent+chunkSize, bytesLeft: bytesLeft-chunkSize, chunkSize: chunkSize)
        }
    }
}
