#include <16F887.h>
#fuses HS, NOWDT, NOLVP
#use delay(clock=20000000)

// Ð?nh nghia chân 
#define servo        PIN_C0   //e1
#define button_up    PIN_E0
#define buttom_dw    PIN_E2
////////////////
#define CHAR_F 0x8E
#define CHAR_U 0xC1
#define CHAR_L 0xC7


////////////////
#define led4 pin_c7
#define led3 pin_c6
#define led2 pin_c5
#define led1 pin_c4           //dn chan quét led

////////////

#define sensor1      pin_a4
#define sensor2      pin_a5
#define sensor4      pin_a1
#define sensor5      pin_a2

const unsigned int8 maled[]={0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x92, 0xc7 };
signed int8 slot,z;
unsigned int8 SW1, SW2, num_pressed, count,i;
// Bi?n toàn c?c
int current_angle = 0;  // Góc hi?n t?i (ban d?u là 0 d?)
// Thêm các bi?n di?u khi?n logic
int door_open = 0;
int direction = 0; // 1 = vào, 2 = ra
int waiting_sensor_clear = 0;
int waiting_pass_sensor = 0;
int sensor_triggered = 0;
int8 prev_sensor4 = 1;
int8 prev_sensor5 = 1;

const unsigned int8 mat_khau_hop_le[] = {1, 2};  // Có th? thêm nhi?u mã khác
unsigned int8 password = 0, key = 0xFF;


unsigned int8 key_4x4()
{
     const unsigned int8 mq[4]={0xef,0xdf,0xbf,0x7f};
     unsigned int8 cot, mp=0xff;
     for(cot=0;cot<4;cot++)
     {
            output_b(mq[cot]);
            while(input(pin_b0)==0) mp = cot*4 + 0;
            while(input(pin_b1)==0) mp = cot*4 + 1;
            while(input(pin_b2)==0) mp = cot*4 + 2;
            while(input(pin_b3)==0) mp = cot*4 + 3;
     }
     return mp;
}



void display_FULL() {
    output_d(CHAR_F);
    output_low(led1);
    delay_ms(1);
    output_high(led1);

    output_d(CHAR_U);
    output_low(led2);
    delay_ms(1);
    output_high(led2);

    output_d(CHAR_L);
    output_low(led3);
    delay_ms(1);
    output_high(led3);

    output_d(CHAR_L);
    output_low(led4);
    delay_ms(1);
    output_high(led4);
}



// ==== T?o xung di?u khi?n servo ====
void servo_write(int angle) {
    int pulse = 1000 + (angle * 1000) / 180;
    output_high(servo);
    delay_us(pulse);         // gi? xung cho servo
    output_low(servo);
    delay_us(20000 - pulse); // gi? chu k? 20ms (servo c?n v?y)
}
// ==== Quay servo d?n v? trí mong mu?n ====
void servo_quay(int angle) {
    {
        servo_write(angle);
    }
    current_angle = angle;
}

// ==== Hàm ki?m tra nút tang (RE0) ====
void check_increase_button() {
    if (!input(button_up)) {
        delay_ms(1); // ch?ng d?i
        if (current_angle == 0) {
            servo_quay(90); // Quay d?n góc 90 d?
        }
    }
}

// ==== Hàm ki?m tra nút gi?m (RE2) ====
void check_decrease_button() {
    if (!input(buttom_dw)) {
        delay_ms(1); // ch?ng d?i
        if (current_angle == 90) {
            servo_quay(0); // Quay v? góc 0 d?
        }
    }
}

  
void slot_xe() {
    SW1 = input(sensor1);
    SW2 = input(sensor2);
    num_pressed = SW1 + SW2 ;
    count = num_pressed;
}


void quet_led() {
    if (count == 0) {
        display_FULL();
    } else {
        output_d(maled[count % 10]);
        output_low(led4);
        delay_ms(1);
        output_high(led4);

        output_d(maled[count / 10 % 10]);
        output_low(led3);
        delay_ms(1);
        output_high(led3);

        output_d(maled[11]);
        output_low(led2);
        delay_ms(1);
        output_high(led2);

        output_d(maled[10]);
        output_low(led1);
        delay_ms(1);
        output_high(led1);
    }
}
void delay_msx(unsigned int16 t)
{
   for(i=0;i<=t;i++)
   {
      delay_ms(1);
      quet_led();
    }
}

int kiem_tra_mat_khau(unsigned int8 mp) {
    for (int i = 0; i < sizeof(mat_khau_hop_le); i++) {
        if (mp == mat_khau_hop_le[i])
            return 1;
    }
    return 0;
}


void nhap_mat_khau() {
    key = key_4x4();
    if (key != 0xFF) {
        if (key >= 0 && key <= 15) {
            if (key == 3) { // phím Enter
                if (kiem_tra_mat_khau(password)) {
                    servo_quay(90);  // M? c?a
                    door_open = 1;
                    waiting_sensor_clear = 1; // Ch? sensor hi?n t?i v? 1 (r?i kh?i vùng sensor)
                }
                password = 0;
            } else {
                password = key;
            }
        }
        delay_ms(1);
    }
}



#int_timer1
void ngat_timer1() {
    quet_led();  // Quét LED trong m?i l?n ng?t
    set_timer1(64911); // N?p l?i giá tr? cho TMR1 (tuong duong ~1ms v?i 20MHz)
}

// ==== Hàm kh?i t?o ====
void init_io() {
    set_tris_e(0xf5); 
    set_tris_c(0x0f);
    set_tris_d(0x00);
    set_tris_a(0xff);
}

void main() {
    init_io();
    setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);
    set_timer1(64911);
    enable_interrupts(INT_TIMER1);
    enable_interrupts(GLOBAL);
    servo_quay(0); // Góc 0 ban d?u

    while(TRUE) {
        slot_xe(); // d?m xe n?u c?n
        int8 sensor4_now = input(sensor4);
        int8 sensor5_now = input(sensor5);

        // Tru?ng h?p c?a dang dóng
        if (!door_open) {
            if (sensor4_now == 0) {
                direction = 1;
                nhap_mat_khau();
            }
            else if (sensor5_now == 0) {
                direction = 2;
                nhap_mat_khau();
            }
        }

        // N?u c?a dã m? và dang ch? ngu?i r?i kh?i sensor kích ho?t (A1 ho?c A2)
        if (door_open && waiting_sensor_clear) {
            if ((direction == 1 && sensor4_now == 1 && prev_sensor4 == 0) || 
                (direction == 2 && sensor5_now == 1 && prev_sensor5 == 0)) {
                waiting_sensor_clear = 0;
                waiting_pass_sensor = 1;  // B?t d?u ch? sensor còn l?i có xung
            }
        }

        // Ð?i xe di qua sensor còn l?i d? dóng c?a
        if (door_open && waiting_pass_sensor) {
            

            if ((direction == 2 && prev_sensor4 == 1 && sensor4_now == 0)) {
                // sensor4 t? 1 ? 0 (ra vùng) b?t d?u d?m xung
            }
            else if (direction == 2 && prev_sensor4 == 0 && sensor4_now == 1) {
                // sensor4 t? 0 ? 1: dã di qua
                servo_quay(0);
                door_open = 0;
                waiting_pass_sensor = 0;
             }
            if ((direction == 1 && prev_sensor5 == 1 && sensor5_now == 0)) {
                // sensor5 t? 1 ? 0 (vào vùng) b?t d?u d?m xung
            }
            else if (direction == 1 && prev_sensor5 == 0 && sensor5_now == 1) {
                // sensor5 t? 0 ? 1: dã di qua
                servo_quay(0);
                door_open = 0;
                waiting_pass_sensor = 0;
            }
       }
        

        prev_sensor4 = sensor4_now;
        prev_sensor5 = sensor5_now;

        check_increase_button();
        check_decrease_button();
    }
}


