#include "writer.h"
#include <math.h>
#include <iostream>

void writeMap(v8::Local<v8::Object> toWrite, Orient::RecordWriter & writer);
void writeArray(v8::Local<v8::Array> toWrite, Orient::RecordWriter & writer);
void writeValue(v8::Local<v8::Value> value, Orient::RecordWriter & writer);
// not supported in node 0.x
//
//void writeBinary(v8::Local<v8::Uint8Array> toWrite, Orient::RecordWriter & writer);

void writeObject(v8::Local<v8::Object> toWrite,Orient::RecordWriter & writer){

	v8::Local<v8::String> classKey = Nan::New("@class").ToLocalChecked();
	if(Nan::Has(toWrite,classKey).FromJust()){
		v8::Local<v8::Value> val = Nan::Get(toWrite,classKey).ToLocalChecked();
		if(val->IsString()){
			Nan::Utf8String clazzVal(val);
			writer.startDocument(*clazzVal);
		}else if(val->IsObject()) {
			v8::Local<v8::String> nameKey = Nan::New("name").ToLocalChecked();
			v8::Local<v8::Object> classObj = val->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
			if(Nan::Has(classObj, nameKey).FromJust()){
				Nan::Utf8String clazzVal(Nan::Get(classObj,nameKey).ToLocalChecked());
				writer.startDocument(*clazzVal);
			}else 
				writer.startDocument("");
		}else 
			writer.startDocument("");
	} else {
		writer.startDocument("");
	}
	v8::Local<v8::Array> properties = toWrite->GetPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();
	unsigned int  i;
	for(i = 0; i< properties->Length() ; i ++){

		v8::Local<v8::String> name = v8::Local<v8::String>::Cast(Nan::Get(properties,i).ToLocalChecked());
		Nan::Utf8String val(Nan::Get(properties,i).ToLocalChecked());
		// v8::String::Utf8Value val(name);
		if((*val)[0] != '@') {
			writer.startField(*val);
			v8::Local<v8::Value> value = Nan::Get(toWrite,name).ToLocalChecked();
			writeValue(value,writer);
			writer.endField(*val);
		}
	}
	writer.endDocument();
}


void writeValue(v8::Local<v8::Value> value, Orient::RecordWriter & writer) {

	if(value->IsString()){
		Nan::Utf8String sval(value);
		writer.stringValue(*sval);
	} else if (value->IsInt32()){
		writer.intValue(value->ToInt32(Nan::GetCurrentContext()).ToLocalChecked()->Value());
	} else if (value->IsNumber()){
		v8::Local<v8::Number> num = value->ToNumber(Nan::GetCurrentContext()).ToLocalChecked();
		double val = num->Value();
		if(ceil(val) != 0 )
			writer.doubleValue(val);
		else
			writer.longValue(val);
	} else if (value->IsDate()){
		long long int date= v8::Local<v8::Date>::Cast(value)->NumberValue(Nan::GetCurrentContext()).FromJust();
		writer.dateTimeValue(date);
	} else if (value->IsNull()){
		writer.nullValue();
	} else if (value->IsBoolean()){
		writer.booleanValue(Nan::To<v8::Boolean>(value).ToLocalChecked()->Value());
	} else if (value->IsArray()){
		writeArray(v8::Local<v8::Array>::Cast(value), writer);
	}  else if (value->IsObject()){
			v8::Local<v8::String> typeKey = Nan::New("@type").ToLocalChecked();
      			//TODO: check if replace with RecordID prototype check
			v8::Local<v8::String> clusterKey = Nan::New("cluster").ToLocalChecked();
			v8::Local<v8::String> positionKey = Nan::New("position").ToLocalChecked();
			v8::Local<v8::String> dVal = Nan::New("d").ToLocalChecked();
			v8::Local<v8::Object> obj = value->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
			if(Nan::Has(obj,typeKey).FromJust() && Nan::Get(obj,typeKey).ToLocalChecked()->Equals(Nan::GetCurrentContext(),dVal).FromJust()){
				writeObject(obj,writer);
			} else if(Nan::Has(obj,clusterKey).FromJust() && Nan::Has(obj,positionKey).FromJust()){
				struct Orient::Link lnk;
				lnk.cluster = Nan::Get(obj,clusterKey).ToLocalChecked()->ToNumber(Nan::GetCurrentContext()).ToLocalChecked()->Value();
				lnk.position = Nan::Get(obj,positionKey).ToLocalChecked()->ToNumber(Nan::GetCurrentContext()).ToLocalChecked()->Value();
				writer.linkValue(lnk);
			} else {
				writeMap(obj,writer);
			}
	}
}


void writeMap(v8::Local<v8::Object> toWrite, Orient::RecordWriter & writer) {
	v8::Local<v8::Array> properties = toWrite->GetPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();
	unsigned int  i;
	writer.startMap(properties->Length(),Orient::EMBEDDEDMAP);
	for(i = 0; i< properties->Length() ; i ++){		
		v8::Local<v8::String> name = v8::Local<v8::String>::Cast(Nan::Get(properties,i).ToLocalChecked());
		Nan::Utf8String val(Nan::Get(properties,i).ToLocalChecked());
		writer.mapKey(*val);
		v8::Local<v8::Value> value = Nan::Get(toWrite,name).ToLocalChecked();
		writeValue(value,writer);
	}
	writer.endMap(Orient::EMBEDDEDMAP);
}


void writeArray(v8::Local<v8::Array> toWrite, Orient::RecordWriter & writer){
	unsigned int  i;
	writer.startCollection(toWrite->Length(),Orient::EMBEDDEDLIST);
	for(i = 0; i< toWrite->Length() ; i ++){
		v8::Local<v8::Value> value = Nan::Get(toWrite,i).ToLocalChecked();
		writeValue(value,writer);
	}
	writer.endCollection(Orient::EMBEDDEDLIST);
}

// Not supported in node 0.x

//void writeBinary(v8::Local<v8::Uint8Array toWrite, Orient::RecordWriter & writer){
//
//	char* buf = node::Buffer::Data(toWrite->ToObject());
//	writer.binaryValue(buf,toWrite->Length());
//}

