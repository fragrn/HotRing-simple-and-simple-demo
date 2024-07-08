#include <iostream>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
using namespace std;
#define inputfile "test5.data"

const int N = 10000000;                  //总访问数
const int M = 1000;                      //总项数

//hotring
class htEntry {
public:
	htEntry(const string &k, const string &v, htEntry *n, unsigned int t, unsigned char o, unsigned char r):
		key(k), val(v), next(n), tag(t), occupied(o), rehash(r) {}

	htEntry() : htEntry("", "", nullptr, 0, 0, 0) {}
	htEntry(const string &k, const string &v, htEntry *n, unsigned int t) : htEntry(k, v, n, t, 0, 0) {}
    htEntry(const string &k, unsigned t) : htEntry(k, "", nullptr, t, 0, 0){}

    string getKey()const;
    void setKey(const string &s);

    string getVal()const;
    void setVal(const string &s);

    htEntry *getNext()const;
    void setNext(htEntry *n);
    
    unsigned int getTag()const;
    void setTag(const unsigned int t);
    
    unsigned char getOccupied()const;
    void setOccupied(const unsigned char o);

    unsigned char getRehash()const;
    void setRehash(const unsigned char r);

    bool operator < (const htEntry &other);
	

private:
	
	string key;                 // 键
	string val;                 // 值

    unsigned int tag;           // tag值
    unsigned char occupied;     // 占用标识，多线程实现时启用
    unsigned char rehash;       // rehash标识

    htEntry *next;            // 指向下个哈希表节点，形成链表
};

class hotring {
public:
    //构造函数
    hotring(unsigned int sz);
    ~hotring();

    //插入
    bool insert(const string &key, const string &val);
    //删除
    bool remove(const string &key);
    //更新
    bool update(const string &key, const string &val);
    //查找
    htEntry *search(const string &key);

    unsigned int getfindcnt();
    
    unsigned int getmaxFindcnt();
    unsigned int getminFindcnt();
private:
    // 计算哈希值的函数
    unsigned int hashFunction(const string &key);

    // 测试所用函数
    void setMinMax(const unsigned int onecnt);

private:
	vector<htEntry*> table;     // 哈希表数组
	unsigned int size;          // 哈希表大小
	unsigned int sizemask;      // 哈希表大小掩码，用于计算索引值 总是等于size-1
    unsigned int r;             // 记录访问次数当r==R时,开始热点转移
    htEntry *compareItem;       // 用于查询时比较

    // 测试所用变量
    const unsigned int R = 5;   // 控制多少次访问后进行热点转移
	unsigned int findcnt;       // 统计总的查找次数
    unsigned int maxFindcnt;    // 统计最大查找次数,一定程度上反应尾延迟
    unsigned int minFindcnt;    // 统计最小查找次数
};


class ht {
public:
    //构造函数
    ht(unsigned int sz);


    //插入
    bool insert(const string &key, const string &val);
    //删除
    bool remove(const string &key);
    //更新
    bool update(const string &key, const string &val);
    //查找
    htEntry *search(const string &key);

    unsigned int getfindcnt();
    unsigned int getmaxFindcnt();
    unsigned int getminFindcnt();
private:
    // 计算哈希值的函数
    unsigned int hashFunction(const string &key);

    // 测试所用函数
    void setMinMax(const unsigned int onecnt);

private:
    vector<htEntry*> table;   // 哈希表数组
    unsigned int size;          // 哈希表大小
    unsigned int sizemask;      // 哈希表大小掩码，用于计算索引值 总是等于size-1
    unsigned int r;             // 记录访问次数当r==R时,开始热点转移

    // 测试所用变量
    const unsigned int R = 5;   // 控制多少次访问后进行热点转移
    unsigned int findcnt;       // 统计总的查找次数
    unsigned int maxFindcnt;    // 统计最大查找次数,一定程度上反应尾延迟
    unsigned int minFindcnt;    // 统计最小查找次数
};


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

bool hotring::insert(const string & key, const string & val)
{
    // 去重
    if (search(key) != nullptr) return false;

    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;
    unsigned int tag = hashValue & (~this->sizemask);
    htEntry *newItem = new htEntry(key, val, nullptr, tag);
    htEntry *pre = nullptr;
    htEntry *nxt = nullptr;

    

    if (table[index] == nullptr) // 环中0项
    {
        table[index] = newItem;
        newItem->setNext(newItem);
    }
    else if (table[index]->getNext() == table[index]) // 环中1项
    {
        if ((*newItem) < (*table[index])) 
        {
            newItem->setNext(table[index]);
            table[index]->setNext(newItem);
        }
        else 
        {
            newItem->setNext(table[index]);
            table[index]->setNext(newItem);
        }
    }
    else
    {
        pre = table[index];
        nxt = table[index]->getNext();
        while (true) {

            if (((*pre) < (*newItem) && (*newItem) < (*nxt)) ||     //ordre_i-1 < order_k < order_i
                ((*newItem) < (*nxt) && (*nxt) < (*pre))     ||     //order_k < order_i < order_i-1
                ((*nxt) < (*pre) && (*pre) < (*newItem)))           //order_i < order_i-1 < order_k
            {
                newItem->setNext(nxt);
                pre->setNext(newItem);
                break;
            }
            nxt = nxt->getNext();
            pre = pre->getNext();
        }
    }
    return true;
}

bool hotring::remove(const string & key)
{
    htEntry *p = search(key);
    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;

    if (p == nullptr) return false;
    htEntry *pre = p;
    while (pre->getNext() != p) 
    {
        pre = pre->getNext();
    }
    pre->setNext(p->getNext());

    // 头指针指向的节点被删除
    if (table[index] == p)
    {
        // 只有一项
        if (pre == p) 
        {
            table[index] = nullptr;
        }
        else 
        {
            table[index] = p->getNext();
        }
    }
    delete p;
    return true;
}

bool hotring::update(const string & key, const string & val)
{
    htEntry *p = search(key);
    if (p == nullptr) return false;
    p->setVal(val);
    return true;
}

htEntry *hotring::search(const string & key)
{
    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;
    unsigned int tag = hashValue & (~this->sizemask);
    htEntry *pre = nullptr;
    htEntry *nxt = nullptr;
    htEntry *res = nullptr;
    unsigned int precnt = findcnt;
    bool hotspotAware = false;
    compareItem->setKey(key);
    compareItem->setTag(tag);

    ++this->r;
    if (this->r == this->R) {
        hotspotAware = true;
        this->r = 0;
    }

    ++this->findcnt;
    if (table[index] == nullptr) // 环中0项
    {
        res = nullptr;
    }
    else if (table[index]->getNext() == table[index]) // 环中1项
    {
        if (key == table[index]->getKey())
        {
            res = table[index];
        }
    }
    else
    {
        pre = table[index];
        nxt = table[index]->getNext();
        while (true) 
        {
            if (pre->getKey() == key) 
            {
                if (hotspotAware) 
                {
                    table[index] = pre;
                }
                res = pre;
                break;
            }

            if (((*pre) < (*compareItem) && (*compareItem) < (*nxt)) ||     //ordre_i-1 < order_k < order_i
                ((*compareItem) < (*nxt) && (*nxt) < (*pre)) ||             //order_k < order_i < order_i-1
                ((*nxt) < (*pre) && (*pre) < (*compareItem)))               //order_i < order_i-1 < order_k
            {
                
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

bool htEntry::operator<(const htEntry &other)
{
    if (this->tag == other.getTag()) 
    {
        return this->key < other.getKey();
    }
    return this->tag < other.getTag();
}

//hashtable


ht::ht(unsigned int sz) :table(0), findcnt(0), minFindcnt(0x7fffffff), maxFindcnt(0)
{
    unsigned int htsz = 1;
    while (htsz < sz) htsz <<= 1;
    this->table.resize(htsz, nullptr);
    size = htsz;
    sizemask = htsz - 1;
}

unsigned int ht::hashFunction(const string & key)
{
    unsigned int hash = 5381;

    for (char c : key)
    {
        hash += (hash << 5) + c;
    }
    //return key[0] << 7;
    return (hash & 0x7FFFFFFF);
}

void ht::setMinMax(const unsigned int onecnt)
{
    this->maxFindcnt = max(this->maxFindcnt, onecnt);
    this->minFindcnt = min(this->minFindcnt, onecnt);
}

bool ht::insert(const string & key, const string & val)
{
    // 去重
    if (search(key) != nullptr) return false;

    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;
    unsigned int tag = hashValue & (~this->sizemask);
    htEntry *newItem = new htEntry(key, val, nullptr, tag);
    htEntry *pre = nullptr;
    htEntry *nxt = nullptr;

    //头插法插入
    newItem->setNext(table[index]);
    table[index] = newItem;
    
    return true;
}

bool ht::remove(const string & key)
{
    htEntry *p = search(key);
    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;

    if (p == nullptr) return false;

    if (table[index] == p)
    {
        table[index] = p->getNext();
    }
    else 
    {
        htEntry *pre = table[index];
        while (pre && pre->getNext() != p)
        {
            pre = pre->getNext();
        }
        pre->setNext(p->getNext());
    }
    delete p;
    return true;
}

bool ht::update(const string & key, const string & val)
{
    htEntry *p = search(key);
    if (p == nullptr) return false;
    p->setVal(val);
    return true;
}

htEntry *ht::search(const string & key)
{
    unsigned int hashValue = hashFunction(key);
    unsigned int index = hashValue & this->sizemask;
    htEntry *p = table[index];
    unsigned int precnt = findcnt;
    bool hotspotAware = false;

    ++this->findcnt;
    while (p && p->getKey() != key) {
        ++this->findcnt;
        p = p->getNext();
    }
    setMinMax(this->findcnt - precnt);
    return p;
}

unsigned int ht::getfindcnt()
{
    return this->findcnt;
}

unsigned int ht::getmaxFindcnt()
{
    return this->maxFindcnt;
}

unsigned int ht::getminFindcnt()
{
    return this->minFindcnt;
}




int main()
{
    time_t start, stop;
    string key;
    string val;
    ht h(M);
    hotring hr(M);



    fstream input(inputfile);

    

    for (int i = 0; i < M; ++i) {
        input >> key >> val;
        h.insert(key, val);
    }

    start = time(NULL);
    while(input >> key){
        h.search(key);
    }
    stop = time(NULL);

    cout << "HashTable:" << endl;
    cout << "findcnt:" << h.getfindcnt() << "次" << endl;
    cout << "maxFindcnt:" << h.getmaxFindcnt() << "次" << endl;
    cout << "minFindcnt:" << h.getminFindcnt() << "次" << endl;
    cout << "averageFindcnt:" << (double)h.getfindcnt() / N << "次" << endl;
    cout << "Use Time:" << (stop - start) << "s" << endl << endl;




    input.close();
    input.open(inputfile);



    for (int i = 0; i < M; ++i) {
        input >> key >> val;
        hr.insert(key, val);
    }

    start = time(NULL);
    while (input >> key) {
        hr.search(key);
    }
    stop = time(NULL);

    cout << "HotRing:" << endl;
    cout << "findcnt:" << hr.getfindcnt() << "次" << endl;
    cout << "maxFindcnt:" << hr.getmaxFindcnt() << "次" << endl;
    cout << "minFindcnt:" << hr.getminFindcnt() << "次" << endl;
    cout << "averageFindcnt:" << (double)hr.getfindcnt() / N << "次" << endl;
    cout << "Use Time:" << (stop - start) << "s" << endl;

    return 0;
}
