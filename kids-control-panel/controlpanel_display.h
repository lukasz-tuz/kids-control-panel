
class Display
{
public:
    const uint8_t digitToSegment[16] = {
        // XGFEDCBA
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111, // 9
        0b01110111, // A
        0b01111100, // b
        0b00111001, // C
        0b01011110, // d
        0b01111001, // E
        0b01110001  // F
    };

    Display(uint8_t size)
    {
        Display::size = size;
    }

    uint8_t map_cahracter(char c)
    {
        // ASCII digits: 0x30 - 0x39
        // ASCII A-F: 0x41 - 0x46
        uint8_t index = 0;

        if (c >= 0x30 and c <= 0x39)
            index = c - 0x30;
        if (c >= 0x41 and c <= 0x46)
            index = c - 0x40 + 9;
        return digitToSegment[index];
    }

    void set_character(char c)
    {
        if (c == '*' or c == '#')
        {
            buffer[buf_ptr] = 0;
        }
        else
        {
            buffer[buf_ptr] = map_cahracter(c);
        }
        ++buf_ptr %= size;
    }

    uint8_t get_ptr()
    {
        return Display::buf_ptr;
    }

    uint8_t *get_buffer()
    {
        return buffer;
    }

private:
    static const uint8_t max_size = 16;
    uint8_t size = 4;
    uint8_t buffer[max_size] = {0};
    uint8_t buf_ptr = 0;
};
