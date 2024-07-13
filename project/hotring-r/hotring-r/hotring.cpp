#include "hotring.h"

hotring::hotring(unsigned int sz):table(0), findcnt(0), minFindcnt(0x7fffffff), maxFindcnt(0)
{
    unsigned int htsz = 1;
    while (htsz < sz) htsz <<= 1;
    this->table.resize(htsz, nullptr);
    size = htsz;
    sizemask = htsz - 1;
    compareItem = new htEntry("", 0);
}

hotring::~hotring()
{
    delete compareItem;
}

unsigned int hotring::hashFunction(const string & key)
{
    unsigned int hash = 5381;

    for(char c:key)
    {
        hash += (hash << 5) + c;
    }
    //return key[0] << 7;
    return (hash & 0x7FFFFFFF);
}

void hotring::setMinMax(const unsigned int onecnt)
{
    this->maxFindcnt = max(this->maxFindcnt, onecnt);
    this->minFindcnt = min(this->minFindcnt, onecnt);
}
bool hotring::R_put(const string & key, const string & val)
{
    // 去重
    // if (R_read(key) != nullptr) return false;

    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;
    unsigned int tag = hashValue & (~this->sizemask);
    htEntry *newItem = new htEntry(key, val, nullptr, tag);
    htEntry *pre = nullptr;
    htEntry *nxt = nullptr;

    // bool hotspotAware = false;
    // ++this->r;
    // if (this->r == this->R) {
    //     hotspotAware = true;
    //     this->r = 0;
    // }

    if (table[index] == nullptr) // 环中0项
    {
        cout<<"0 item"<<endl;
        table[index] = newItem;
        newItem->setNext(newItem);
        cout<<"0 item sloved"<<endl;
    }
    else if (table[index]->getNext() == table[index]) // 环中1项
    {
        cout<<"1 item"<<endl;
        if((*newItem)==(*table[index])){
            table[index]->setVal(newItem->getVal());
        }
        else if ((*newItem) < (*table[index])) 
        {
            newItem->setNext(table[index]);
            table[index]->setNext(newItem);
        }
        else 
        {   
            table[index]->setNext(newItem);
            newItem->setNext(table[index]);
        }
        cout<<"1 item solved"<<endl;
    }
    else
    {
        pre = table[index];
        nxt = table[index]->getNext();
        while (!((*nxt)==(*table[index]))) {
            if((*pre)==(*newItem)){
                // pre->setVal(newItem->getVal());
                // newItem=pre;
                cout<<"hit it"<<endl;
                break;
            }
            if (((*pre) < (*newItem) && (*newItem) < (*nxt)) ||     //ordre_i-1 < order_k < order_i
                ((*newItem) < (*nxt) && (*nxt) < (*pre))     ||     //order_k < order_i < order_i-1
                ((*nxt) < (*pre) && (*pre) < (*newItem)))           //order_i < order_i-1 < order_k
            {
                cout<<"put not found"<<endl;
                newItem->setNext(nxt);
                pre->setNext(newItem);
                break;
            }
            nxt = nxt->getNext();
            pre = pre->getNext();
        }
        // if(hotspotAware){
        //     table[index]=newItem;//热点转移
        // }
    }
    return true;
}


// bool hotring::remove(const string & key)
// {
//     htEntry *p = R_search(key);
//     unsigned int hashValue = hashFunction(key);
//     unsigned int index = hashValue & this->sizemask;

//     if (p == nullptr) return false;
//     htEntry *pre = p;
//     while (pre->getNext() != p) 
//     {
//         pre = pre->getNext();
//     }
//     pre->setNext(p->getNext());

//     // 头指针指向的节点被删除
//     if (table[index] == p)
//     {
//         // 只有一项
//         if (pre == p) 
//         {
//             table[index] = nullptr;
//         }
//         else 
//         {
//             table[index] = p->getNext();
//         }
//     }
//     delete p;
//     return true;
// }

// bool hotring::update(const string & key, const string & val)
// {
//     htEntry *p = R_search(key);
//     if (p == nullptr) return false;
//     p->setVal(val);
//     return true;
// }

htEntry *hotring::R_read(const string & key)
{
    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;
    unsigned int tag = hashValue & (~this->sizemask);
    htEntry *pre = nullptr;
    htEntry *nxt = nullptr;
    htEntry *res = nullptr;
    unsigned int precnt = findcnt;
    bool hotspotAware = false;
    compareItem->setKey(key);  //compareItem为当前这次search的key
    compareItem->setTag(tag);

    ++this->r;
    if (this->r == this->R) {
        hotspotAware = true;
        this->r = 0;
    }

    ++this->findcnt;
    if (table[index] == nullptr) // 环中0项
    {
        cout<<"read,0 item"<<endl;
        res = nullptr;
    }
    else if (table[index]->getNext() == table[index]) // 环中1项
    {
        cout<<"read, 1 item"<<endl;
        if (key == table[index]->getKey())
        {
            res = table[index];
        }
        else{
            res=nullptr;
        }
    }
    else
    {
        pre = table[index];
        nxt = table[index]->getNext();
        cout<<"read: enter the read loop"<<endl;
        while (true) 
        {
            // if (pre->getKey() == key) 
            if((*pre)==(*compareItem))
            {
                cout<<"read, hit it"<<endl;
                if (hotspotAware) 
                {
                    table[index] = pre;
                    //热点转移，hot access和cold access都可以这样实现，
                    //如果是hot access则多了一次table[index]=table[index]的赋值，不影响性能
                }
                res = pre;
                break;
            }

            if (((*pre) < (*compareItem) && (*compareItem) < (*nxt)) ||     //ordre_i-1 < order_k < order_i
                ((*compareItem) < (*nxt) && (*nxt) < (*pre)) ||             //order_k < order_i < order_i-1
                //此时nxt在链表头部，pre在尾部，这便是这个环遍历一遍的结束判断条件
                ((*nxt) < (*pre) && (*pre) < (*compareItem)))               //order_i < order_i-1 < order_k
                //此时nxt在链表头部，pre在尾部
            {
                cout<<"read not hit"<<endl;
                res = nullptr;
                break;
            }
            nxt = nxt->getNext();
            pre = pre->getNext();
            ++this->findcnt;
        }
    }
    setMinMax(this->findcnt - precnt);
    return res;
}


unsigned int hotring::getfindcnt()
{
    return this->findcnt;
}

unsigned int hotring::getmaxFindcnt()
{
    return this->maxFindcnt;
}

unsigned int hotring::getminFindcnt()
{
    return this->minFindcnt;
}

string htEntry::getKey()const
{
    return this->key;
}

void htEntry::setKey(const string & s)
{
    this->key = s;
}

string htEntry::getVal()const
{
    return this->val;
}

void htEntry::setVal(const string & s)
{
    this->val = s;
}

htEntry * htEntry::getNext()const
{
    return this->next;
}

void htEntry::setNext(htEntry * n)
{
    this->next = n;
}

unsigned int htEntry::getTag()const
{
    return this->tag;
}

void htEntry::setTag(const unsigned int t)
{
    this->tag = t;
}

unsigned char htEntry::getOccupied()const
{
    return this->occupied;
}

void htEntry::setOccupied(const unsigned char o)
{
    this->occupied = o;
}

unsigned char htEntry::getRehash()const
{
    return this->rehash;
}

void htEntry::setRehash(const unsigned char r)
{
    this->rehash = r;
}

bool htEntry::operator<(const htEntry &other)//先比tag，后比key
{
    if (this->tag == other.getTag()) 
    {
        return this->key < other.getKey();
    }
    return this->tag < other.getTag();
}
bool htEntry::operator==(const htEntry &other){
    return this->key==other.getKey();
}