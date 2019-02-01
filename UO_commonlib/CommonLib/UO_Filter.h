#ifndef FILTER_H_UO
#define FILTER_H_UO

#include <stdint.h>
#include <iostream>
#include <bitset>
#include "UO_Public.h"

class UO_Hash_Table
{
public:
	UO_Hash_Table(uint32_t num = 1024):m_num(num),m_total(num * 10),m_hashednum(0)
	{
		Conflict = 0;
		_InitCryptTable();
		m_phashnode = new HASHNODE[m_total];
		memset(m_phashnode,0,sizeof(HASHNODE) * m_total);
	}
	~UO_Hash_Table()
	{
		if(m_phashnode)
			delete []m_phashnode;
	}

	bool Add_To_HashTable(char *name,void *value)
	{
		if(m_hashednum == m_num)
			return false;
		uint32_t hash = _HashKey(name,0);
		uint32_t hash1 = _HashKey(name,1);
		uint32_t hash2 = _HashKey(name,2);
		uint32_t posstart = hash % m_total;
		uint32_t pos = posstart;

		while(m_phashnode[pos].value)
		{
			if(m_phashnode[pos].hash1 == hash1 && m_phashnode[pos].hash2 == hash2)
				return false;
			Conflict++;
			pos = (++pos) % m_total;
			if(pos == posstart)
				return false;
		}
		m_phashnode[pos].hash1 = hash1;
		m_phashnode[pos].hash2 = hash2;
		m_phashnode[pos].value = value;
		++m_hashednum;
		return true;
	}
	uint32_t Conflict;
private:
	struct HASHNODE
	{
		uint32_t hash1;
		uint32_t hash2;
		void *value;
	};
	HASHNODE *m_phashnode;
	uint32_t m_num;
	uint32_t m_total;
	uint32_t m_hashednum;
	uint32_t cryptTable[0x500];
	//from Blizzard
	uint32_t _HashKey(char *pName, uint32_t dwHashType = 0)
	{
		char *key = pName;
		uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
		int ch;
		while(*key != 0)
		{
			ch = toupper(*key++);
			seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}
		return seed1;
	}

	void _InitCryptTable()  
	{
		uint32_t seed = 0x00100001, index1 = 0, index2 = 0, i;  
		for( index1 = 0; index1 < 0x100; index1++ )  
		{   
			for( index2 = index1, i = 0; i < 5; i++, index2 += 0x100 )  
			{   
				uint32_t temp1, temp2;  
				seed = (seed * 125 + 3) % 0x2AAAAB;  
				temp1 = (seed & 0xFFFF) << 0x10;  
				seed = (seed * 125 + 3) % 0x2AAAAB;  
				temp2 = (seed & 0xFFFF);  
				cryptTable[index2] = ( temp1 | temp2 );   
			}   
		}   
	}
};

class UO_BitMap
{
public:
	UO_BitMap(uint32_t num = 10240){inibitset(num);}
	~UO_BitMap()
	{
		if(m_pbitmap)
			delete []m_pbitmap;
	}

	bool add_to_bitmap(uint32_t pos)
	{
		uint32_t i = pos >> 10;
		uint32_t j = pos & 0x400;
		if(m_pbitmap[i].test(j))
			return false;
		m_pbitmap[i].set(j);
		return true;
	}
	bool delete_from_bitmap(uint32_t pos)
	{
		uint32_t i = pos >> 10;
		uint32_t j = pos & 0x400;
		if(!m_pbitmap[i].test(j))
			return false;
		m_pbitmap[i].reset(j);
		return true;
	}
	bool if_in_bitmap(uint32_t pos)
	{
		uint32_t i = pos >> 10;
		uint32_t j = pos & 0x400;
		if(!m_pbitmap[i].test(j))
			return false;
		return true;
	}

private:
	void inibitset(uint32_t num)
	{
		uint32_t bitsetnum = 1;
		if(num > 1024)
			bitsetnum = num >> 10;
		m_pbitmap = new std::bitset<1024>[bitsetnum];
		for(int i = 0;i < bitsetnum;i++)
			m_pbitmap[i].reset();
	}
	std::bitset<1024> *m_pbitmap;
};

#endif
