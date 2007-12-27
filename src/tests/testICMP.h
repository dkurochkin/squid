#ifndef SQUID_SRC_TEST_URL_H
#define SQUID_SRC_TEST_URL_H

#include <cppunit/extensions/HelperMacros.h>

#include "ICMP.h"

#if USE_ICMP

class stubICMP : public ICMP
{
public:
    stubICMP() {};
    virtual ~stubICMP() {};
    virtual int Open() { return 0; };
    virtual void Close() {};

    /// Construct ECHO request
    virtual void SendEcho(IPAddress &to, int opcode, const char *payload, int len) {};

    /// Handle ICMP responses.
    virtual void Recv(void) {};

/* methods to relay test data from tester to private methods being tested */
    int testChecksum(unsigned short *ptr, int size) { return CheckSum(ptr,size); };
    int testHops(int ttl) { return ipHops(ttl); };
};

#endif /* USE_ICMP */

/**
 * test the ICMP base class.
 */
class testICMP : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( testICMP );
#if USE_ICMP
    CPPUNIT_TEST( testChecksum );
    CPPUNIT_TEST( testHops );
#endif /* USE_ICMP */
    CPPUNIT_TEST_SUITE_END();

protected:
#if USE_ICMP
    void testChecksum();
    void testHops();
#endif /* USE_ICMP */
};

#endif