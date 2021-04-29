/**
 * @brief 
 * 
 */

class Display
{
public:
    // segments rotated by 90deg
    const uint8_t digitToSegment[16] = {
        //XGFEDCBA
        0b00111111, // 0
        0b00110000, // 1
        0b01011011, // 2
        0b01111001, // 3
        0b01110100, // 4
        0b01101101, // 5
        0b01101111, // 6
        0b00111000, // 7
        0b01111111, // 8
        0b01111101, // 9
        0b01111110, // A
        0b01100111, // b
        0b00001111, // C
        0b01110011, // d
        0b01111001, // E
        0b01110001  // F
    };

    /**
     * @brief Construct a new Display object
     * 
     * @param size Number of characters in the display.
     */
    Display(uint8_t size)
    {
        Display::size = size;
    }

    /**
     * @brief Map character into 7seg display segments.
     * 
     * @param c Character to be converted, [0-9A-Z] range
     * @return uint8_t Bit map with segments to be turned on for specified character.
     */
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

    /**
     * @brief Writes the character c to display buffer
     * 
     * @param c The Character.
     */
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

    /**
     * @brief Clears character buffer and resets pointer to 0.
     * 
     */
    void clear()
    {
        Display::buffer[max_size] = {0};
        Display::buf_ptr = 0;
    }

    /**
     * @brief Get the disaplay buffer pointer
     * 
     * @return uint8_t Number representing current place in the display
     */
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
