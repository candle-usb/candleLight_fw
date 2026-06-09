#include <check.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Pull in the descriptor size constants from the production headers */
#include "usbd_gs_can.h"
#include "usbd_def.h"

START_TEST(test_descriptor_fits_in_desc_buf)
{
    /* Invariant: every descriptor copied into USBD_DescBuf must be
       strictly <= USB_MAX_EP0_SIZE (the declared buffer size) so that
       no memcpy can overflow the destination. */

    struct {
        const char *name;
        size_t      desc_size;
    } payloads[] = {
        /* exact exploit case: CfgDesc is the largest descriptor */
        { "CfgDesc",              sizeof(USBD_GS_CAN_CfgDesc)            },
        /* boundary: WinUSB string descriptor */
        { "WINUSB_STR",           sizeof(USBD_GS_CAN_WINUSB_STR)         },
        /* MS compat-id feature descriptor */
        { "MS_COMP_ID",           sizeof(USBD_MS_COMP_ID_FEATURE_DESC)   },
        /* MS extended-property feature descriptor */
        { "MS_EXT_PROP",          sizeof(USBD_MS_EXT_PROP_FEATURE_DESC)  },
    };
    int num_payloads = (int)(sizeof(payloads) / sizeof(payloads[0]));

    for (int i = 0; i < num_payloads; i++) {
        /* The destination buffer used by the production memcpy calls is
           USBD_DescBuf, which is declared as uint8_t USBD_DescBuf[USB_MAX_EP0_SIZE].
           Assert that every source descriptor fits inside that buffer. */
        ck_assert_msg(
            payloads[i].desc_size <= USB_MAX_EP0_SIZE,
            "Descriptor '%s' size %zu exceeds USBD_DescBuf capacity %u — "
            "buffer overflow in usbd_gs_can.c memcpy",
            payloads[i].name,
            payloads[i].desc_size,
            (unsigned)USB_MAX_EP0_SIZE
        );
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s       = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_descriptor_fits_in_desc_buf);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int      number_failed;
    Suite   *s;
    SRunner *sr;

    s  = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}