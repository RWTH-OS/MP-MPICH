//---------------------------------------------------------------------------
#ifndef AccountManagerH
#define AccountManagerH
#include <vcl\Classes.hpp>

#include <map>
//---------------------------------------------------------------------------

BOOL WINAPI AccountCallback(char*,char*,char*,char*,void*);

class TCred : public TObject {
public:
	TCred(const char *n,const char *d,const char *p,bool per=false) :Name(n),Domain(d),Password(p) {
    	Name = Name.Trim().UpperCase();
        Domain=Domain.Trim().UpperCase();
        Password = Password.Trim();
        deleted=false;
        Persistent=per;
    }
    TCred() {deleted = Persistent=false;}
    AnsiString &GetName() {return Name;}
    AnsiString &GetDomain() {return Domain;}
    AnsiString &GetPassword() {return Password;}
    operator == (TCred &o) { return ((Name.UpperCase() ==o.Name) &&
                                     (Domain.UpperCase() == o.Domain) &&
                                     (Password == o.Password));}
    void SetDeleted(bool d=true) {deleted=d;}
    bool IsDeleted() {return deleted;}
    bool Persistent;
private:
	AnsiString Name,Domain,Password;
    bool deleted;

};

typedef std::map<AnsiString,TCred*, std::less<AnsiString> > TAccountMap;
typedef TAccountMap::iterator TAccountIndex;

class TAccountManager {
friend BOOL WINAPI AccountCallback(char*,char*,char*,char*,void*);

public:
	TAccountManager();
    ~TAccountManager();
    TAccountIndex StoreAccount(AnsiString Name, TCred *Values);
    TAccountIndex StoreAccount(const char *Name, const char *User, const char *Domain, const char *Password) {
    	return StoreAccount(AnsiString(Name),new TCred(User,Domain,Password));
    }
    BOOL DeleteAccount(const char *Name){ return DeleteAccount(AnsiString(Name)); }
    BOOL DeleteAccount(AnsiString Name);
    BOOL DeleteAccount(TAccountIndex &index);
	TCred *GetAccount(AnsiString Name);
   	TCred *GetAccount(char *Name) { return GetAccount(AnsiString(Name)); }
   	TCred *GetAccount(TAccountIndex &index, AnsiString &AccountName);
    TAccountIndex FindAccount(AnsiString &Uname,AnsiString &Domain);
    TAccountIndex GetFirstIndex() {return Accounts.begin();}
    TAccountIndex InsertAccount(AnsiString &AccountName,TCred *Values);
    BOOL SetActualAccount(AnsiString Name);
    BOOL SetActualAccount(AnsiString User,AnsiString Domain);
    BOOL SetActualAccount(TAccountIndex id);
    TCred *GetActualAccount();
    BOOL IsActual(TAccountIndex &id) {return id==Current;}
    BOOL IsActual(AnsiString name) {
    	return ((Current != Accounts.end()) && (name==(*Current).first));
    }

protected:
	TAccountIndex AddAccount(AnsiString &AccountName,TCred *Values);

private:
	TAccountMap Accounts;
    TAccountIndex Current;
};

extern BOOL GetActualUser(AnsiString &UName,AnsiString &Domain);
extern TAccountManager AccountManager;
#endif
