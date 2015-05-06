#include <unittest++/UnitTest++.h>
#include <unittest++/TestMacros.h>
#include "common.h"
#include "database_test.h"
/*
TEST(sha512){
  vector<hash512_t> hashes = {
    Id("wj01lEv8X4PwlP2Rciyn"),
    Id("InFDcKbYZD7NDISCPkGP"),
    Id("p891dIuOJuSL7em7IVJ1"),
    Id("DHHI1eocea3Mv4lPIC0O")
  };
  vector<const char*> corrects = {
    "A4E8E064447CF8AC931A3B3C6951CBF34C7314C90AC777837001F75C67DE3A512505746A7B1BCAADCC49C2411D4AC126FFF2FDA4C8E19BC2B52B40BF0753EA94",
    "1FC0A12D0F96298441A92255840C24BDECE98831C28D726CD2C36225C164B413C708CC451D10B2C63F7A6B4C958AF3F48549CC736D4CB8C8472A1567A374DA35",
    "2763B9A7B8B6C028A464A6F98543B59E0829F0575F588C9421D57890539204F50B6C2CE2E4F8308A60B954805CC831D58003096F659D90BE5F043979B025E20C",
    "4E543466F3BDDE6AB0F7537F03E0D1630533FE18841DCB01A3FF3299B9454C090860AE9A758B9FAEE05A957BE49B0C859A17E617B76F3654E8B34704DC7C0485"
  };

  stringstream ss;
 //to string
 for (int i = 0; i < 4; i++){
    ss << hashes[i];
    CHECK_EQUAL(ss.str().c_str(), corrects[i]);
    ss.str(string());
    ss.clear();
  }

 //from string
 for (int i = 0; i < 4; i++){
   Id test_id;
   test_id.from_string(corrects[i]);
   CHECK_EQUAL(hashes[i], test_id);
 }
}
*/

int main()
{
  return UnitTest::RunAllTests();
}
