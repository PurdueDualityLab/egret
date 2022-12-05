//
// Created by charlie on 12/2/22.
//

#include <gtest/gtest.h>
#include "egret/egret.h"

TEST(LongSlow, test_one) {
  const std::string regex = "{\"access_token\":\"c170f85d-9371-31c3-a4fa-1f0865ef28da\",\"refresh_token\":\"6312ab1a-e257-38e9-aab1-8f8071dfd643\",\"scope\":\"openid\",\"id_token\":\"eyJ4NXQiOiJOVEF4Wm1NeE5ETXlaRGczTVRVMVpHTTBNekV6T0RKaFpXSTRORE5sWkRVMU9HRmtOakZpTVEiLCJraWQiOiJOVEF4Wm1NeE5ETXlaRGczTVRVMVpHTTBNekV6T0RKaFpXSTRORE5sWkRVMU9HRmtOakZpTVEiLCJhbGciOiJSUzI1NiJ9.eyJhdF9oYXNoIjoibTFGZUk4ZFJFbDV6azZoTFd2VUxBUSIsImNfaGFzaCI6IkRGbVY3RUt4dzlhck4xNFhDOUxQT1EiLCJzdWIiOiJzb2wxdXNlcjYwIiwiYXVkIjpbInRlc3RtYW5lZXNoYWNvZGUwMDAwMDAxMjMwMDAwIl0sImF6cCI6InRlc3RtYW5lZXNoYWNvZGUwMDAwMDAxMjMwMDAwIiwiYW1yIjpbIkJhc2ljQXV0aGVudGljYXRvciJdLCJpc3MiOiJodHRwczpcL1wvbG9jYWxob3N0Ojk0NDNcL29hdXRoMlwvdG9rZW4iLCJleHAiOjE1MzM3Mjk4MjYsImlhdCI6MTUzMzcyNjIyNiwic2lkIjoiZmE0NDNjZTMtNTI0MS00N2ZkLWIxYzYtMDkxNTU1ODNmZDcwIn0.J6ymae3s4UOvNRiJIL_kdni4HRgUoIHf4speGduQjGd_ny_3ow0MjxHW51wxJfAWpSw9a8cMfc5DkI0lgQmPoWUeNlQ1-loNOGPCMREE4PfJPG312psjxecIjU0j7fr8ma-wAkPKMicSK1VrrbURgU9XdVjve3iVOyAW7ypuGuIgOqwulQdYMldTVBrixWjQvjmygezV30V48OAY8oNIrp6nLWzBUV9w9Zb7_ASblLSezMceIecHMl-0EBAnMYvqgxtbk8aB* Connection #0 to host localhost left intact tLOsz3bqAonEMk3EvnWrQ7GsL2Mrtc_Wo2n7bfv3fkdjkeeb8fuVqUuC4nG-u2In9hdck8HOXoojZQ\",\"token_type\":\"Bearer\",\"expires_in\":2881}";
  // TODO check for a timeout somehow
  EXPECT_NO_FATAL_FAILURE(run_engine(regex, "evil"));
}
