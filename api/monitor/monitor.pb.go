// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.28.1
// 	protoc        v3.21.12
// source: monitor.proto

package monitor

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

// -------------------------------------------------
// |             Metadata Definition               |
// -------------------------------------------------
type StatusMetadata struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	FreeMem   string `protobuf:"bytes,1,opt,name=FreeMem,proto3" json:"FreeMem,omitempty"`
	DiskUsage string `protobuf:"bytes,2,opt,name=DiskUsage,proto3" json:"DiskUsage,omitempty"`
}

func (x *StatusMetadata) Reset() {
	*x = StatusMetadata{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StatusMetadata) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StatusMetadata) ProtoMessage() {}

func (x *StatusMetadata) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StatusMetadata.ProtoReflect.Descriptor instead.
func (*StatusMetadata) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{0}
}

func (x *StatusMetadata) GetFreeMem() string {
	if x != nil {
		return x.FreeMem
	}
	return ""
}

func (x *StatusMetadata) GetDiskUsage() string {
	if x != nil {
		return x.DiskUsage
	}
	return ""
}

// -------------------------------------------------
// |                API Definition                 |
// -------------------------------------------------
type ConnectRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	KernelVer string `protobuf:"bytes,1,opt,name=KernelVer,proto3" json:"KernelVer,omitempty"`
	// Distribution version
	DistVer   string          `protobuf:"bytes,2,opt,name=DistVer,proto3" json:"DistVer,omitempty"`
	Arch      string          `protobuf:"bytes,3,opt,name=Arch,proto3" json:"Arch,omitempty"`
	SchedPort string          `protobuf:"bytes,4,opt,name=SchedPort,proto3" json:"SchedPort,omitempty"`
	Stat      *StatusMetadata `protobuf:"bytes,5,opt,name=Stat,proto3" json:"Stat,omitempty"`
}

func (x *ConnectRequest) Reset() {
	*x = ConnectRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ConnectRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ConnectRequest) ProtoMessage() {}

func (x *ConnectRequest) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ConnectRequest.ProtoReflect.Descriptor instead.
func (*ConnectRequest) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{1}
}

func (x *ConnectRequest) GetKernelVer() string {
	if x != nil {
		return x.KernelVer
	}
	return ""
}

func (x *ConnectRequest) GetDistVer() string {
	if x != nil {
		return x.DistVer
	}
	return ""
}

func (x *ConnectRequest) GetArch() string {
	if x != nil {
		return x.Arch
	}
	return ""
}

func (x *ConnectRequest) GetSchedPort() string {
	if x != nil {
		return x.SchedPort
	}
	return ""
}

func (x *ConnectRequest) GetStat() *StatusMetadata {
	if x != nil {
		return x.Stat
	}
	return nil
}

type ConnectResponse struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	ID string `protobuf:"bytes,1,opt,name=ID,proto3" json:"ID,omitempty"`
}

func (x *ConnectResponse) Reset() {
	*x = ConnectResponse{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *ConnectResponse) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*ConnectResponse) ProtoMessage() {}

func (x *ConnectResponse) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use ConnectResponse.ProtoReflect.Descriptor instead.
func (*ConnectResponse) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{2}
}

func (x *ConnectResponse) GetID() string {
	if x != nil {
		return x.ID
	}
	return ""
}

type StatUpdateRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Stat *StatusMetadata `protobuf:"bytes,1,opt,name=Stat,proto3" json:"Stat,omitempty"`
}

func (x *StatUpdateRequest) Reset() {
	*x = StatUpdateRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StatUpdateRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StatUpdateRequest) ProtoMessage() {}

func (x *StatUpdateRequest) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StatUpdateRequest.ProtoReflect.Descriptor instead.
func (*StatUpdateRequest) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{3}
}

func (x *StatUpdateRequest) GetStat() *StatusMetadata {
	if x != nil {
		return x.Stat
	}
	return nil
}

type StatUpdateResponse struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields
}

func (x *StatUpdateResponse) Reset() {
	*x = StatUpdateResponse{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[4]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *StatUpdateResponse) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*StatUpdateResponse) ProtoMessage() {}

func (x *StatUpdateResponse) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[4]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use StatUpdateResponse.ProtoReflect.Descriptor instead.
func (*StatUpdateResponse) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{4}
}

type DisconnectRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	ID string `protobuf:"bytes,1,opt,name=ID,proto3" json:"ID,omitempty"`
}

func (x *DisconnectRequest) Reset() {
	*x = DisconnectRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[5]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *DisconnectRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*DisconnectRequest) ProtoMessage() {}

func (x *DisconnectRequest) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[5]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use DisconnectRequest.ProtoReflect.Descriptor instead.
func (*DisconnectRequest) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{5}
}

func (x *DisconnectRequest) GetID() string {
	if x != nil {
		return x.ID
	}
	return ""
}

type DisconnectResponse struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields
}

func (x *DisconnectResponse) Reset() {
	*x = DisconnectResponse{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[6]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *DisconnectResponse) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*DisconnectResponse) ProtoMessage() {}

func (x *DisconnectResponse) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[6]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use DisconnectResponse.ProtoReflect.Descriptor instead.
func (*DisconnectResponse) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{6}
}

type TaskFinishRequest struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	ID   string `protobuf:"bytes,1,opt,name=ID,proto3" json:"ID,omitempty"`
	File []byte `protobuf:"bytes,2,opt,name=File,proto3" json:"File,omitempty"`
}

func (x *TaskFinishRequest) Reset() {
	*x = TaskFinishRequest{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[7]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TaskFinishRequest) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TaskFinishRequest) ProtoMessage() {}

func (x *TaskFinishRequest) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[7]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TaskFinishRequest.ProtoReflect.Descriptor instead.
func (*TaskFinishRequest) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{7}
}

func (x *TaskFinishRequest) GetID() string {
	if x != nil {
		return x.ID
	}
	return ""
}

func (x *TaskFinishRequest) GetFile() []byte {
	if x != nil {
		return x.File
	}
	return nil
}

type TaskFinishResponse struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields
}

func (x *TaskFinishResponse) Reset() {
	*x = TaskFinishResponse{}
	if protoimpl.UnsafeEnabled {
		mi := &file_monitor_proto_msgTypes[8]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TaskFinishResponse) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TaskFinishResponse) ProtoMessage() {}

func (x *TaskFinishResponse) ProtoReflect() protoreflect.Message {
	mi := &file_monitor_proto_msgTypes[8]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TaskFinishResponse.ProtoReflect.Descriptor instead.
func (*TaskFinishResponse) Descriptor() ([]byte, []int) {
	return file_monitor_proto_rawDescGZIP(), []int{8}
}

var File_monitor_proto protoreflect.FileDescriptor

var file_monitor_proto_rawDesc = []byte{
	0x0a, 0x0d, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12,
	0x07, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x22, 0x48, 0x0a, 0x0e, 0x53, 0x74, 0x61, 0x74,
	0x75, 0x73, 0x4d, 0x65, 0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x12, 0x18, 0x0a, 0x07, 0x46, 0x72,
	0x65, 0x65, 0x4d, 0x65, 0x6d, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x07, 0x46, 0x72, 0x65,
	0x65, 0x4d, 0x65, 0x6d, 0x12, 0x1c, 0x0a, 0x09, 0x44, 0x69, 0x73, 0x6b, 0x55, 0x73, 0x61, 0x67,
	0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x09, 0x44, 0x69, 0x73, 0x6b, 0x55, 0x73, 0x61,
	0x67, 0x65, 0x22, 0xa7, 0x01, 0x0a, 0x0e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x52, 0x65,
	0x71, 0x75, 0x65, 0x73, 0x74, 0x12, 0x1c, 0x0a, 0x09, 0x4b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x56,
	0x65, 0x72, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x09, 0x4b, 0x65, 0x72, 0x6e, 0x65, 0x6c,
	0x56, 0x65, 0x72, 0x12, 0x18, 0x0a, 0x07, 0x44, 0x69, 0x73, 0x74, 0x56, 0x65, 0x72, 0x18, 0x02,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x07, 0x44, 0x69, 0x73, 0x74, 0x56, 0x65, 0x72, 0x12, 0x12, 0x0a,
	0x04, 0x41, 0x72, 0x63, 0x68, 0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x41, 0x72, 0x63,
	0x68, 0x12, 0x1c, 0x0a, 0x09, 0x53, 0x63, 0x68, 0x65, 0x64, 0x50, 0x6f, 0x72, 0x74, 0x18, 0x04,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x09, 0x53, 0x63, 0x68, 0x65, 0x64, 0x50, 0x6f, 0x72, 0x74, 0x12,
	0x2b, 0x0a, 0x04, 0x53, 0x74, 0x61, 0x74, 0x18, 0x05, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x17, 0x2e,
	0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0x4d, 0x65,
	0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x52, 0x04, 0x53, 0x74, 0x61, 0x74, 0x22, 0x21, 0x0a, 0x0f,
	0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x12,
	0x0e, 0x0a, 0x02, 0x49, 0x44, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x02, 0x49, 0x44, 0x22,
	0x40, 0x0a, 0x11, 0x53, 0x74, 0x61, 0x74, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 0x52, 0x65, 0x71,
	0x75, 0x65, 0x73, 0x74, 0x12, 0x2b, 0x0a, 0x04, 0x53, 0x74, 0x61, 0x74, 0x18, 0x01, 0x20, 0x01,
	0x28, 0x0b, 0x32, 0x17, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x53, 0x74, 0x61,
	0x74, 0x75, 0x73, 0x4d, 0x65, 0x74, 0x61, 0x64, 0x61, 0x74, 0x61, 0x52, 0x04, 0x53, 0x74, 0x61,
	0x74, 0x22, 0x14, 0x0a, 0x12, 0x53, 0x74, 0x61, 0x74, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 0x52,
	0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x22, 0x23, 0x0a, 0x11, 0x44, 0x69, 0x73, 0x63, 0x6f,
	0x6e, 0x6e, 0x65, 0x63, 0x74, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12, 0x0e, 0x0a, 0x02,
	0x49, 0x44, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x02, 0x49, 0x44, 0x22, 0x14, 0x0a, 0x12,
	0x44, 0x69, 0x73, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e,
	0x73, 0x65, 0x22, 0x37, 0x0a, 0x11, 0x54, 0x61, 0x73, 0x6b, 0x46, 0x69, 0x6e, 0x69, 0x73, 0x68,
	0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x12, 0x0e, 0x0a, 0x02, 0x49, 0x44, 0x18, 0x01, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x02, 0x49, 0x44, 0x12, 0x12, 0x0a, 0x04, 0x46, 0x69, 0x6c, 0x65, 0x18,
	0x02, 0x20, 0x01, 0x28, 0x0c, 0x52, 0x04, 0x46, 0x69, 0x6c, 0x65, 0x22, 0x14, 0x0a, 0x12, 0x54,
	0x61, 0x73, 0x6b, 0x46, 0x69, 0x6e, 0x69, 0x73, 0x68, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73,
	0x65, 0x32, 0x9c, 0x02, 0x0a, 0x07, 0x4d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x12, 0x3c, 0x0a,
	0x07, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x12, 0x17, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74,
	0x6f, 0x72, 0x2e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73,
	0x74, 0x1a, 0x18, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x43, 0x6f, 0x6e, 0x6e,
	0x65, 0x63, 0x74, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x12, 0x45, 0x0a, 0x0a, 0x53,
	0x74, 0x61, 0x74, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 0x12, 0x1a, 0x2e, 0x6d, 0x6f, 0x6e, 0x69,
	0x74, 0x6f, 0x72, 0x2e, 0x53, 0x74, 0x61, 0x74, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 0x52, 0x65,
	0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x1b, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e,
	0x53, 0x74, 0x61, 0x74, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e,
	0x73, 0x65, 0x12, 0x45, 0x0a, 0x0a, 0x44, 0x69, 0x73, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74,
	0x12, 0x1a, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x44, 0x69, 0x73, 0x63, 0x6f,
	0x6e, 0x6e, 0x65, 0x63, 0x74, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x1a, 0x1b, 0x2e, 0x6d,
	0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x44, 0x69, 0x73, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63,
	0x74, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x12, 0x45, 0x0a, 0x0a, 0x54, 0x61, 0x73,
	0x6b, 0x46, 0x69, 0x6e, 0x69, 0x73, 0x68, 0x12, 0x1a, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f,
	0x72, 0x2e, 0x54, 0x61, 0x73, 0x6b, 0x46, 0x69, 0x6e, 0x69, 0x73, 0x68, 0x52, 0x65, 0x71, 0x75,
	0x65, 0x73, 0x74, 0x1a, 0x1b, 0x2e, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x2e, 0x54, 0x61,
	0x73, 0x6b, 0x46, 0x69, 0x6e, 0x69, 0x73, 0x68, 0x52, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65,
	0x42, 0x0b, 0x5a, 0x09, 0x2e, 0x2f, 0x6d, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x62, 0x06, 0x70,
	0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_monitor_proto_rawDescOnce sync.Once
	file_monitor_proto_rawDescData = file_monitor_proto_rawDesc
)

func file_monitor_proto_rawDescGZIP() []byte {
	file_monitor_proto_rawDescOnce.Do(func() {
		file_monitor_proto_rawDescData = protoimpl.X.CompressGZIP(file_monitor_proto_rawDescData)
	})
	return file_monitor_proto_rawDescData
}

var file_monitor_proto_msgTypes = make([]protoimpl.MessageInfo, 9)
var file_monitor_proto_goTypes = []interface{}{
	(*StatusMetadata)(nil),     // 0: monitor.StatusMetadata
	(*ConnectRequest)(nil),     // 1: monitor.ConnectRequest
	(*ConnectResponse)(nil),    // 2: monitor.ConnectResponse
	(*StatUpdateRequest)(nil),  // 3: monitor.StatUpdateRequest
	(*StatUpdateResponse)(nil), // 4: monitor.StatUpdateResponse
	(*DisconnectRequest)(nil),  // 5: monitor.DisconnectRequest
	(*DisconnectResponse)(nil), // 6: monitor.DisconnectResponse
	(*TaskFinishRequest)(nil),  // 7: monitor.TaskFinishRequest
	(*TaskFinishResponse)(nil), // 8: monitor.TaskFinishResponse
}
var file_monitor_proto_depIdxs = []int32{
	0, // 0: monitor.ConnectRequest.Stat:type_name -> monitor.StatusMetadata
	0, // 1: monitor.StatUpdateRequest.Stat:type_name -> monitor.StatusMetadata
	1, // 2: monitor.Monitor.Connect:input_type -> monitor.ConnectRequest
	3, // 3: monitor.Monitor.StatUpdate:input_type -> monitor.StatUpdateRequest
	5, // 4: monitor.Monitor.Disconnect:input_type -> monitor.DisconnectRequest
	7, // 5: monitor.Monitor.TaskFinish:input_type -> monitor.TaskFinishRequest
	2, // 6: monitor.Monitor.Connect:output_type -> monitor.ConnectResponse
	4, // 7: monitor.Monitor.StatUpdate:output_type -> monitor.StatUpdateResponse
	6, // 8: monitor.Monitor.Disconnect:output_type -> monitor.DisconnectResponse
	8, // 9: monitor.Monitor.TaskFinish:output_type -> monitor.TaskFinishResponse
	6, // [6:10] is the sub-list for method output_type
	2, // [2:6] is the sub-list for method input_type
	2, // [2:2] is the sub-list for extension type_name
	2, // [2:2] is the sub-list for extension extendee
	0, // [0:2] is the sub-list for field type_name
}

func init() { file_monitor_proto_init() }
func file_monitor_proto_init() {
	if File_monitor_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_monitor_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StatusMetadata); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*ConnectRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*ConnectResponse); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StatUpdateRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[4].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*StatUpdateResponse); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[5].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*DisconnectRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[6].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*DisconnectResponse); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[7].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TaskFinishRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_monitor_proto_msgTypes[8].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TaskFinishResponse); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_monitor_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   9,
			NumExtensions: 0,
			NumServices:   1,
		},
		GoTypes:           file_monitor_proto_goTypes,
		DependencyIndexes: file_monitor_proto_depIdxs,
		MessageInfos:      file_monitor_proto_msgTypes,
	}.Build()
	File_monitor_proto = out.File
	file_monitor_proto_rawDesc = nil
	file_monitor_proto_goTypes = nil
	file_monitor_proto_depIdxs = nil
}
