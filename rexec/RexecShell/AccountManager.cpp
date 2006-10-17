//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop
#include "RexecClient.h"
#include "AccountManager.h"
//---------------------------------------------------------------------------

TAccountManager AccountManager;

//-----------------Helpers---------------------------------------------------
BOOL GetActualUser(AnsiString &UName,AnsiString &Domain) {
	HANDLE hThreadToken;
	TOKEN_USER *pTokenUser;
	DWORD dwActSize,USize=255,DSize = 255;
    char UserName[255],DomainName[255];
    BOOL res;
    SID_NAME_USE NameUse;

	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hThreadToken)) {
	    return FALSE;
	}

	dwActSize=0;
	pTokenUser=0;
	GetTokenInformation(hThreadToken,TokenUser,pTokenUser,0,&dwActSize);

	if(dwActSize) pTokenUser=(TOKEN_USER*)malloc(dwActSize);
	else pTokenUser=0;

	if(!pTokenUser||!GetTokenInformation(hThreadToken,TokenUser,pTokenUser,dwActSize,&dwActSize) ){
		if(hThreadToken) CloseHandle(hThreadToken);
		if(pTokenUser) free(pTokenUser);
		return FALSE;
	}

    if(!LookupAccountSid(0,pTokenUser->User.Sid,
	UserName,&USize,
	DomainName,&DSize,
	&NameUse)) res=FALSE;
    else {
    	UName = UserName;
        UName = UName.UpperCase();
        Domain = DomainName;
        Domain = Domain.UpperCase();
    	res = TRUE;
    }
	CloseHandle(hThreadToken);
	if(pTokenUser) free(pTokenUser);
	return res;

}

//---------------------------------------------------------------------------

BOOL WINAPI AccountCallback(char* Name,char* User,char* Domain ,char* Password ,void* Manager) {
    TAccountManager *M=(TAccountManager*)Manager;
	TCred *values = new TCred(User,Domain,Password,true);
    M->AddAccount(AnsiString(Name),values);
    return TRUE;
}

TAccountManager::TAccountManager() {
    AnsiString User,ADomain,CurName;
	EnumStoredAccounts(AccountCallback,this);
    if(GetActualUser(User,ADomain)) {
		Current = FindAccount(User,ADomain);
	    if(Current == Accounts.end()) {
    		CurName = ADomain.UpperCase() + AnsiString("/")+ User.UpperCase();
			Current = AddAccount(CurName,new TCred(User.c_str(),ADomain.c_str(),""));
	    }
    } else Current = Accounts.end();
}

TAccountManager::~TAccountManager() {
	TAccountIndex id;
    for(id=Accounts.begin(); id!=Accounts.end();++id)
    	delete (*id).second;
}

TAccountIndex TAccountManager::StoreAccount(AnsiString Name, TCred *V) {
	BOOL res;
    if(!V) return Accounts.end();
	TAccountIndex id = Accounts.find(Name);
    if(id != Accounts.end()) {
     if(*V == *(*id).second && (*id).second->Persistent) {
        (*id).second->SetDeleted(false);
    	return id;
     } else {
		res = ::StoreAccount(Name.c_str(),V->GetName().c_str(),
                                          V->GetDomain().c_str(),
                                          V->GetPassword().c_str());
     	if(res) {
			*(*id).second = *V;
 			(*id).second->Persistent = true;
            (*id).second->SetDeleted(false);
            delete V;
        }
     }
    } else {
	    res = ::StoreAccount(Name.c_str(),V->GetName().c_str(),
                                          V->GetDomain().c_str(),
                                          V->GetPassword().c_str());
    	if(res) {
        	id=Accounts.insert(TAccountMap::value_type(Name,V)).first;
            (*id).second->Persistent = true;
        }
        else
        {
        DWORD errnum=0;
        BOOL result;
        char name[256];
        //storing account failed   si 13.10.2003
        errnum=GetLastError();
        Application->MessageBox((char*)GetLastErrorText(result,name,256)
           ,"Saving account failed",MB_OK);

        }
     }
     return id;
}

BOOL TAccountManager::DeleteAccount(AnsiString Name) {
	TAccountIndex id = Accounts.find(Name);
    BOOL res=TRUE;
	if((*id).second->Persistent) {;
		res = ::DeleteAccount(Name.c_str());
        (*id).second->Persistent=false;
    }
    if(id != Current) Accounts.erase(id);
    //else (*id).second->SetDeleted();
    return res;
}

BOOL TAccountManager::DeleteAccount(TAccountIndex &index) {
	if(index != Accounts.end())
		return DeleteAccount((*index).first);
    else return FALSE;
}

TCred* TAccountManager::GetAccount(AnsiString Name) {
	TAccountIndex id = Accounts.find(Name);
	return GetAccount(id,Name);
}

TCred* TAccountManager::GetAccount(TAccountIndex &index, AnsiString &AccountName) {
	if(index != Accounts.end()) {
    	AccountName = (*index).first;
    	return (*index).second;
    } else return 0;
}

TAccountIndex TAccountManager::InsertAccount(AnsiString &Name,TCred *V) {
	if(!V) return Accounts.end();
	TAccountIndex id = Accounts.find(Name.UpperCase());
    if(id != Accounts.end()) {
     	if(*V == *(*id).second) return id;
    	else {
			*(*id).second = *V;
			(*id).second->SetDeleted(false);
            delete V;
     	}
    } else {
		return Accounts.insert(TAccountMap::value_type(Name.UpperCase(),V)).first;
    }
    return id;
}

TAccountIndex TAccountManager::FindAccount(AnsiString &Uname,AnsiString &Domain) {
	TAccountIndex id;
    for(id=Accounts.begin(); id!=Accounts.end();++id) {
		if(Uname.UpperCase() == (*id).second->GetName() &&
           Domain.UpperCase() == (*id).second->GetDomain()&&
           !(*id).second->IsDeleted()) {
           return id;
        }
    }
    return Accounts.end();
}

TCred *TAccountManager::GetActualAccount() {
	if(Current != Accounts.end())
		return (*Current).second;
    else return 0;
}

BOOL TAccountManager::SetActualAccount(AnsiString Name) {
	TAccountIndex f = Accounts.find(Name);
    if(f != Accounts.end()) {
    	Current = f;
        return TRUE;
    } else {
    	return FALSE;
    }
}
BOOL TAccountManager::SetActualAccount(AnsiString User,AnsiString Domain) {
    AnsiString AName;
	TAccountIndex f = FindAccount(User,Domain);
    if(f != Accounts.end()) {
    	Current = f;
        return TRUE;
    } else {
    	return FALSE;
    }
}
BOOL TAccountManager::SetActualAccount(TAccountIndex id) {
	if(id != Accounts.end()) {
		Current = id;
    	return TRUE;
    }
    return FALSE;
}


TAccountIndex TAccountManager::AddAccount(AnsiString &AccountName,TCred *Values) {
	return Accounts.insert(TAccountMap::value_type(AccountName.UpperCase(),Values)).first;
}

