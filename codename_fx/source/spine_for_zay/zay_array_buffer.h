#pragma once

#include "zay_types.h"
#include "zay_image.h"

namespace ZAY
{
    template<typename T>
    class ArrayBuffer
    {
    public:
        ArrayBuffer()
        {
            _owner = nullptr;
            _buffer = nullptr;
            _bufferSize = 0;
            _bufferCapacity = 0;
        }
        
        ~ArrayBuffer()
        {
            clearBuffer();
        }

    public:
        void referenceBuffer(T* buffer, int32_t bufferSize, Base* owner)
        {
            if (bufferSize < 0)
                return;
            
            if (_buffer != buffer)
            {
                if (_buffer &&
                    (_owner == nullptr))
                {
                    delete [] _buffer;
                    _buffer = nullptr;
                }
                
                if (_owner)
                {
                    _owner->release();
                }
                
                _buffer = buffer;
                _bufferSize = bufferSize;
                _bufferCapacity = bufferSize;
                _owner = owner;
                
                if (_owner)
                {
                    _owner->retain();
                }
            }
            else if ((_buffer == buffer) &&
                     _buffer &&
                     (_owner == owner))
            {
                _bufferSize = bufferSize;
                _bufferCapacity = bufferSize;
            }
        }
        
        void createBuffer(int32_t bufferSize, bool setEmptyToZero = false, bool copyOriginal = false)
        {
            if (bufferSize < 0)
                return;
            
            if (_owner)
            {
                _owner->release();
                _owner = nullptr;
                
                _buffer = nullptr;
                _bufferSize = 0;
                _bufferCapacity = 0;
            }
            
            if (_bufferCapacity != bufferSize)
            {
                T* newBuffer = nullptr;
                
                if (bufferSize > 0)
                {
                    newBuffer = new T[bufferSize];
                    
                    if (copyOriginal && _buffer)
                    {
                        if (_bufferSize < bufferSize)
                        {
                            memcpy(newBuffer, _buffer, sizeof(T) * _bufferSize);
                            
                            if (setEmptyToZero)
                            {
                                memset(newBuffer+_bufferSize, 0x00, sizeof(T) * (bufferSize - _bufferSize));
                            }
                        }
                        else
                        {
                            memcpy(newBuffer, _buffer, sizeof(T) * bufferSize);
                        }
                    }
                    else
                    {
                        if (setEmptyToZero)
                        {
                            memset(newBuffer, 0x00, sizeof(T) * bufferSize);
                        }
                    }
                }
                
                if (_buffer)
                {
                    assert(_owner == nullptr);
                    
                    delete [] _buffer;
                }
                
                _buffer = newBuffer;
                _bufferSize = bufferSize;
                _bufferCapacity = bufferSize;
            }
            else if (_buffer &&
                     _bufferCapacity &&
                     _bufferCapacity == bufferSize)
            {
                if (setEmptyToZero)
                {
                    memset(_buffer, 0x00, _bufferCapacity);
                }
                _bufferSize = bufferSize;
            }
        }
        
        void clearBuffer()
        {
            if (_owner)
            {
                _owner->release();
                _owner = nullptr;
                
                _buffer = nullptr;
                _bufferSize = 0;
            }
            else
            {
                if (_buffer)
                {
                    delete [] _buffer;
                    _buffer = nullptr;
                }
                
                _bufferSize = 0;
            }
        }
        
    public:
        Base* getBufferOwner() const
        {
            return _owner;
        }

        void setBufferSizeInCapacity(int32_t bufferSize)
        {
            _bufferSize = std::max(0, std::min(_bufferCapacity, bufferSize));
        }

        int32_t getBufferSize() const
        {
            return _bufferSize;
        }

        int32_t getBufferCapacity() const
        {
            return _bufferCapacity;
        }

        T* getBufferPointer() const
        {
            return _buffer;
        }

    private:
        Base* _owner;
        T* _buffer;
        int32_t _bufferSize;
        int32_t _bufferCapacity;
    };
}
