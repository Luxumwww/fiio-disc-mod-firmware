package main

import (
	"context"
	"encoding/json"
	"fmt"
	"github.com/godbus/dbus/v5"
	"github.com/godbus/dbus/v5/prop"
	"log"
	"os"
	"os/exec"
	"os/signal"
	"runtime"
	"runtime/debug"
	"strings"
	"sync"
	"syscall"
	"time"
)

const serviceUUID = "38ca0001-45b0-4f50-9daa-2caf32da6000"
const commandUUID = "38ca0002-45b0-4f50-9daa-2caf32da6000"
const replyUUID = "38ca0003-45b0-4f50-9daa-2caf32da6000"
const rootPath dbus.ObjectPath = "/disc/remote"
const servicePath dbus.ObjectPath = "/disc/remote/service0"
const commandPath dbus.ObjectPath = "/disc/remote/service0/command"
const replyPath dbus.ObjectPath = "/disc/remote/service0/reply"

type ble struct {
	conn       *dbus.Conn
	engine     *engine
	mu         sync.Mutex
	buffer     []byte
	device     dbus.ObjectPath
	at         time.Time
	mtu        int
	notifying  bool
	requests   chan Request
	properties *prop.Properties
}

func (b *ble) objects() map[dbus.ObjectPath]map[string]map[string]dbus.Variant {
	return map[dbus.ObjectPath]map[string]map[string]dbus.Variant{
		servicePath: {"org.bluez.GattService1": {"UUID": dbus.MakeVariant(serviceUUID), "Primary": dbus.MakeVariant(true)}},
		commandPath: {"org.bluez.GattCharacteristic1": {"UUID": dbus.MakeVariant(commandUUID), "Service": dbus.MakeVariant(servicePath), "Flags": dbus.MakeVariant([]string{"write", "encrypt-write"})}},
		replyPath:   {"org.bluez.GattCharacteristic1": {"UUID": dbus.MakeVariant(replyUUID), "Service": dbus.MakeVariant(servicePath), "Flags": dbus.MakeVariant([]string{"read", "notify", "encrypt-read"}), "Value": dbus.MakeVariant([]byte{3, '{', '}'})}},
	}
}
func (b *ble) GetManagedObjects() (map[dbus.ObjectPath]map[string]map[string]dbus.Variant, *dbus.Error) {
	return b.objects(), nil
}

type command struct{ b *ble }

func (c *command) WriteValue(data []byte, options map[string]dbus.Variant) *dbus.Error {
	device, ok := options["device"].Value().(dbus.ObjectPath)
	if !ok {
		return dbus.NewError("org.bluez.Error.NotAuthorized", nil)
	}
	// BlueZ enforces encrypt-write before calling this method. Dual-mode
	// Android can bond its public identity while Device1 here is a private LE
	// address; checking that temporary object's Paired flag rejects valid links.
	if len(data) == 0 || len(data) > 512 {
		return dbus.NewError("org.bluez.Error.InvalidValueLength", nil)
	}
	b := c.b
	b.mu.Lock()
	defer b.mu.Unlock()
	if b.device != device || time.Since(b.at) > 5*time.Second {
		b.buffer = nil
	}
	b.device = device
	b.at = time.Now()
	if value, ok := options["mtu"]; ok {
		if mtu, ok := value.Value().(uint16); ok {
			b.mtu = int(mtu)
		}
	}
	b.buffer = append(b.buffer, data...)
	if len(b.buffer) > 4096 {
		b.buffer = nil
		return dbus.NewError("org.bluez.Error.InvalidValueLength", nil)
	}
	if data[len(data)-1] != '\n' {
		return nil
	}
	var req Request
	err := json.Unmarshal(b.buffer, &req)
	b.buffer = nil
	if err != nil {
		return dbus.NewError("org.bluez.Error.InvalidArguments", nil)
	}
	log.Printf("BLE request id=%d op=%s", req.ID, req.Op)
	select {
	case b.requests <- req:
		return nil
	default:
		return dbus.NewError("org.bluez.Error.InProgress", nil)
	}
}

type reply struct{ b *ble }

func (r *reply) ReadValue(options map[string]dbus.Variant) ([]byte, *dbus.Error) {
	return []byte("DISC Remote 0.1.1"), nil
}
func (r *reply) StartNotify() *dbus.Error {
	r.b.mu.Lock()
	r.b.notifying = true
	r.b.mu.Unlock()
	return nil
}
func (r *reply) StopNotify() *dbus.Error {
	r.b.mu.Lock()
	r.b.notifying = false
	r.b.mu.Unlock()
	return nil
}
func (b *ble) send(v map[string]any) {
	log.Printf("BLE reply id=%v ok=%v", v["id"], v["ok"])
	data, _ := json.Marshal(v)
	if len(data) > 16000 {
		data = []byte(`{"id":0,"ok":false,"error":"response too large"}`)
	}
	b.mu.Lock()
	size := b.mtu - 4
	if size < 19 {
		size = 19
	}
	if size > 240 {
		size = 240
	}
	active := b.notifying
	b.mu.Unlock()
	if !active {
		return
	}
	first := true
	for len(data) > 0 {
		n := size
		if n > len(data) {
			n = len(data)
		}
		flags := byte(0)
		if first {
			flags |= 1
		}
		if n == len(data) {
			flags |= 2
		}
		value := append([]byte{flags}, data[:n]...)
		b.properties.SetMust("org.bluez.GattCharacteristic1", "Value", value)
		data = data[n:]
		first = false
		time.Sleep(8 * time.Millisecond)
	}
}

type advertisement struct{}

func (*advertisement) Release() *dbus.Error { return nil }

type agent struct{}

func (*agent) Release() *dbus.Error                                                   { return nil }
func (*agent) Cancel() *dbus.Error                                                    { return nil }
func (*agent) RequestConfirmation(device dbus.ObjectPath, passkey uint32) *dbus.Error { return nil }
func (*agent) RequestAuthorization(device dbus.ObjectPath) *dbus.Error                { return nil }
func (*agent) AuthorizeService(device dbus.ObjectPath, uuid string) *dbus.Error       { return nil }
func (*agent) DisplayPasskey(device dbus.ObjectPath, passkey uint32, entered uint16) *dbus.Error {
	return nil
}
func (*agent) RequestPasskey(device dbus.ObjectPath) (uint32, *dbus.Error) {
	return 0, dbus.NewError("org.bluez.Error.Rejected", nil)
}
func (*agent) RequestPinCode(device dbus.ObjectPath) (string, *dbus.Error) {
	return "", dbus.NewError("org.bluez.Error.Rejected", nil)
}
func properties(conn *dbus.Conn, path dbus.ObjectPath, iface string, values map[string]dbus.Variant) (*prop.Properties, error) {
	m := map[string]*prop.Prop{}
	for k, v := range values {
		m[k] = &prop.Prop{Value: v.Value(), Writable: false, Emit: prop.EmitConst}
	}
	if iface == "org.bluez.GattCharacteristic1" {
		m["Value"] = &prop.Prop{Value: []byte{}, Writable: false, Emit: prop.EmitTrue}
	}
	return prop.Export(conn, path, map[string]map[string]*prop.Prop{iface: m})
}
func (b *ble) export() error {
	if e := b.conn.Export(b, rootPath, "org.freedesktop.DBus.ObjectManager"); e != nil {
		return e
	}
	for path, ifs := range b.objects() {
		for iface, values := range ifs {
			p, e := properties(b.conn, path, iface, values)
			if e != nil {
				return e
			}
			if path == replyPath {
				b.properties = p
			}
		}
	}
	b.conn.Export(&command{b}, commandPath, "org.bluez.GattCharacteristic1")
	b.conn.Export(&reply{b}, replyPath, "org.bluez.GattCharacteristic1")
	advPath := dbus.ObjectPath("/disc/remote/advertisement")
	b.conn.Export(&advertisement{}, advPath, "org.bluez.LEAdvertisement1")
	_, e := properties(b.conn, advPath, "org.bluez.LEAdvertisement1", map[string]dbus.Variant{"Type": dbus.MakeVariant("peripheral"), "ServiceUUIDs": dbus.MakeVariant([]string{serviceUUID}), "LocalName": dbus.MakeVariant("DISC Remote")})
	if e != nil {
		return e
	}
	return b.conn.Export(&agent{}, "/disc/remote/agent", "org.bluez.Agent1")
}
func (b *ble) register(ctx context.Context) error {
	adapter := b.conn.Object("org.bluez", "/org/bluez/hci0")
	if e := adapter.CallWithContext(ctx, "org.freedesktop.DBus.Properties.Set", 0, "org.bluez.Adapter1", "Powered", dbus.MakeVariant(true)).Err; e != nil {
		return e
	}
	if e := adapter.CallWithContext(ctx, "org.bluez.GattManager1.RegisterApplication", 0, rootPath, map[string]dbus.Variant{}).Err; e != nil && !strings.Contains(e.Error(), "Already Exists") {
		return e
	}
	if e := adapter.CallWithContext(ctx, "org.bluez.LEAdvertisingManager1.RegisterAdvertisement", 0, dbus.ObjectPath("/disc/remote/advertisement"), map[string]dbus.Variant{}).Err; e != nil && !strings.Contains(e.Error(), "Already Exists") {
		return e
	}
	manager := b.conn.Object("org.bluez", "/org/bluez")
	manager.CallWithContext(ctx, "org.bluez.AgentManager1.RegisterAgent", 0, dbus.ObjectPath("/disc/remote/agent"), "NoInputNoOutput")
	manager.CallWithContext(ctx, "org.bluez.AgentManager1.RequestDefaultAgent", 0, dbus.ObjectPath("/disc/remote/agent"))
	return nil
}
func main() {
	runtime.GOMAXPROCS(1)
	debug.SetMemoryLimit(12 << 20)
	debug.SetGCPercent(50)
	syscall.Setpriority(syscall.PRIO_PROCESS, 0, 10)
	conn, e := dbus.ConnectSystemBus()
	if e != nil {
		log.Fatal(e)
	}
	defer conn.Close()
	b := &ble{conn: conn, engine: newEngine("/usr/data/disc-runtime/module-data/disc-remote-control"), mtu: 23, requests: make(chan Request, 8)}
	if e = b.export(); e != nil {
		log.Fatal(e)
	}
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGTERM, syscall.SIGINT)
	defer stop()
	go func() {
		for {
			select {
			case <-ctx.Done():
				return
			case req := <-b.requests:
				b.send(b.engine.handle(req))
			}
		}
	}()
	// A RAM diagnostic endpoint uses exactly the same parser/engine. It is never
	// network-exposed and permits host integration tests before phone acceptance.
	go func() {
		last := ""
		for {
			select {
			case <-ctx.Done():
				return
			case <-time.After(100 * time.Millisecond):
				data, _ := os.ReadFile("/tmp/disc-remote-request")
				if len(data) == 0 || len(data) > 4096 || string(data) == last {
					continue
				}
				last = string(data)
				var req Request
				if json.Unmarshal(data, &req) != nil {
					continue
				}
				os.Remove("/tmp/disc-remote-request")
				reply, _ := json.Marshal(b.engine.handle(req))
				os.WriteFile("/tmp/disc-remote-reply.next", reply, 0600)
				os.Rename("/tmp/disc-remote-reply.next", "/tmp/disc-remote-reply")
			}
		}
	}()
	registeredOwner := ""
	var daemonAttempt time.Time
	ticker := time.NewTicker(2 * time.Second)
	defer ticker.Stop()
	for {
		owner := ""
		err := conn.BusObject().Call("org.freedesktop.DBus.GetNameOwner", 0, "org.bluez").Store(&owner)
		if err != nil {
			registeredOwner = ""
			os.Remove("/tmp/disc-remote-ready")
			if time.Since(daemonAttempt) < 10*time.Second {
				select {
				case <-ctx.Done():
					return
				case <-ticker.C:
				}
				continue
			}
			daemonAttempt = time.Now()
			command := exec.Command("/usr/bin/bluetoothd", "-n")
			command.Stdout = os.Stdout
			command.Stderr = os.Stderr
			if e := command.Start(); e == nil {
				go command.Wait()
			}
			time.Sleep(time.Second)
		} else {
			powered, pe := conn.Object("org.bluez", "/org/bluez/hci0").GetProperty("org.bluez.Adapter1.Powered")
			if pe != nil || powered.Value() != true {
				registeredOwner = ""
			}
			if owner != registeredOwner {
				attempt, cancel := context.WithTimeout(ctx, 5*time.Second)
				err = b.register(attempt)
				cancel()
				if err == nil {
					registeredOwner = owner
					os.WriteFile("/tmp/disc-remote-ready", []byte(fmt.Sprintf("0.8.1 %d\n", os.Getpid())), 0600)
					log.Print("BLE control ready")
				} else {
					log.Print("BLE register: ", err)
				}
			}
		}
		select {
		case <-ctx.Done():
			b.engine.eq.close()
			os.Remove("/tmp/disc-remote-ready")
			return
		case <-ticker.C:
		}
	}
}
