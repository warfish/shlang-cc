SECTIONS
{
    .test :
    {
        _test_start = .;
        KEEP(*(.test));
        _test_end = .;
    }
}
INSERT AFTER .rodata;