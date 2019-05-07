#include <nan.h>
#include "orientc.h"
#include "listener.h"
#include "writer.h"
#include <iostream>

void Deserialize(const Nan::FunctionCallbackInfo<v8::Value>& info){
  
  int len =node::Buffer::Length(info[0]);
  char * content =  node::Buffer::Data(info[0]);

  
  Orient::RecordParser reader("onet_ser_v0");

  v8::Local<v8::Function> ridFactory= v8::Local<v8::Function>::Cast(info[1]);
  v8::Local<v8::Function> bagFactory= v8::Local<v8::Function>::Cast(info[2]);
  v8::Local<v8::Function> decimalFactory= v8::Local<v8::Function>::Cast(info[3]);
  v8::Local<v8::Boolean> useRidbag= Nan::To<v8::Boolean>(info[4]).ToLocalChecked();
  TrackerListener listener(ridFactory,bagFactory,decimalFactory ,useRidbag->Value());
  reader.parse((unsigned char *)content,len,listener);


  info.GetReturnValue().Set(listener.obj);
}


void Serialize(const Nan::FunctionCallbackInfo<v8::Value>& info){
  v8::Local<v8::Object> toWrite = v8::Local<v8::Object>::Cast(info[0]);
  Orient::RecordWriter writer("onet_ser_v0");

  writeObject(toWrite,writer);
  int size=0;
  const unsigned char * content = writer.writtenContent(&size);
  
  v8::Local<v8::Object> value = Nan::NewBuffer((char *)content,size).ToLocalChecked();
  info.GetReturnValue().Set(value);
}

void Init(v8::Local<v8::Object> exports,v8::Local<v8::Object> module) {
  

  Nan::Set(exports,Nan::New("deserialize").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Deserialize)).ToLocalChecked());
  Nan::Set(exports,Nan::New("serialize").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Serialize)).ToLocalChecked());
}

NODE_MODULE(deserializer, Init)
