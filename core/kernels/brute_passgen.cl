/* 
 * Copyright (C) 2014 Jan Schmied, Radek Hranicky
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 */

#pragma OPENCL EXTENSION cl_amd_printf : enable

#ifndef MAX_PASS_LEN
#define MAX_PASS_LEN 20
#endif

/**
 * @author Honza
 */
kernel void brute_passgen(global uchar* passwords, ulong step, uint max_size, constant uchar *chars, uint chars_count, constant ulong *powers, constant uchar *char_pos) {
    int id = get_global_id(0);

    uchar state[MAX_PASS_LEN] = {0};
    uchar passlen = passwords[(max_size)*id];
    global uchar *password = passwords+((max_size)*id)+1;
    uchar first_char = MAX_PASS_LEN - passlen;
    ulong state_change = step;
    uchar lowest = MAX_PASS_LEN-1;
    uchar carry = 0;

    for(int i = 0;i<passlen;i++){
        state[first_char+i] = char_pos[password[i]];
    }

    while(state_change > 0){
        
        int toAdd,canAdd;
        canAdd = 0;
        // prevedeme co muzeme pricist bez preteceni, do soustavy se kterou se dobre pocita
        for(int i = first_char;i<=lowest;i++){
            canAdd += ((chars_count-1)-state[i])*powers[lowest-i];//prevod z n-kove soustavy do 10kove
        }
        if(state_change > canAdd)
            toAdd = canAdd;
        else
            toAdd = state_change;
        
        state_change -= toAdd;
        
        // pricteme co muzeme
        carry = 0;
        for(int i = lowest;toAdd>0 || carry;i--){
            char state_add = toAdd % chars_count;
            toAdd /= chars_count;
            state[i]+=state_add + carry;
            if(state[i] >= chars_count){
                carry = 1;
                state[i] %= chars_count;
            }else{
                carry = 0;
            }
        }
        
        // pokud stale neco zbyvame, nechame to pretect
        if(state_change > 0){
            state[lowest]++;
            state_change--;
            carry = 0;
            for(int i = lowest;i>=0;i--){
                if(i<first_char){
                    first_char = i;
                    state[i]--;
                }
                state[i] += carry;
                if(state[i]>chars_count-1){
                    state[i] %= chars_count;
                    carry = 1;
                }else{
                    break;
                }
            }
        }

    }
    
    int out_i, in_i;
    for(out_i=0, in_i=first_char;in_i<MAX_PASS_LEN;in_i++,out_i++){
        password[out_i] = chars[state[in_i]];
    }
    passwords[(max_size)*id] = out_i;
}
