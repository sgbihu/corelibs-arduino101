#ifndef __POSTPONEEVENTBUFFER_H__
#define __POSTPONEEVENTBUFFER_H__

template<typename T> class PostponeEventBuffer{
public:
    PostponeEventBuffer(int size);
    ~PostponeEventBuffer();
    bool addEvent(T* event);
    void removeEvent(T* event);
    T* getEvent();
    
private:
    T* allocEventData();
    void freeEventData(T* event);
    
    void setBit(int index){usage_bit_map[index >> 3] |= (1<<(index & 0x07));};
    bool testBit(int index){return usage_bit_map[index >> 3] & (1<<(index & 0x07));};
    void clearBit(int index){usage_bit_map[index >> 3] &= ~(1<<(index & 0x07));};
    
private:
    T* buffer;
    int buffer_size;
    uint8_t* usage_bit_map;
};


template<typename T> PostponeEventBuffer<T>::PostponeEventBuffer(int size): buffer_size(size)
{
    buffer = (T*)malloc(size * sizeof(T));
    usage_bit_map = (uint8_t*)malloc(size + (sizeof(uint8_t) * 8) - 1);
}

template<typename T> PostponeEventBuffer<T>::~PostponeEventBuffer()
{
    free(buffer);
    free(usage_bit_map);
}

template<typename T> T* PostponeEventBuffer<T>::allocEventData()
{
    int i;
    for (i = 0; i < buffer_size; i++)
    {
        if (testBit(i) == false)
        {
            break;
        }
    }
    
    if (i >= buffer_size)
    {
        return NULL;
    }
    
    setBit(i);
    return buffer + i;
}

template<typename T> void PostponeEventBuffer<T>::freeEventData(T* event)
{
    int index = event - buffer;
    if (0 > index || index >= buffer_size)
    {
        return;
    }
    clearBit(index);
}

template<typename T> bool PostponeEventBuffer<T>::addEvent(T* event)
{
    T* local = allocEventData();
    if (NULL == local)
    {
        return false;
    }
    memcpy(local, event, sizeof(T));
    return true;
}

template<typename T> void PostponeEventBuffer<T>::removeEvent(T* event)
{
    freeEventData(event);
}

template<typename T> T* PostponeEventBuffer<T>::getEvent()
{
    int i;
    for (i = 0; i < buffer_size; i++)
    {
        if (testBit(i) == true)
        {
            break;
        }
    }
    
    if (i >= buffer_size)
    {
        return NULL;
    }
    
    return buffer + i;
}

#endif
