/*
 * utility.c
 *
 * Created: 12-01-2017 10:14:53
 *  Author: JP
 */ 

unsigned short f_hex_to_bin(char *hex_in) {
	unsigned short data = 0;
	unsigned char i;
	char cbyte;

	for(i=0; i<4; i++)
	{
		cbyte = *(hex_in+i);
		if ( cbyte >= 'A') cbyte = (cbyte - 'A') + 10;
		else cbyte-= '0';
		data |= (cbyte << (4*(3-i)));
	}

	return data;
}

void f_bin_to_hex(unsigned char bin_in, char *str_out) {
	static char lut_Hex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	*(str_out+0) = lut_Hex[(bin_in >> 4) & 0x0F];
	*(str_out+1) = lut_Hex[bin_in & 0x0F];
}

unsigned char f_hex_byte_to_bin(char *hex_in) {
	unsigned char data = 0;
	unsigned char i;
	char cbyte;

	for(i=0; i<2; i++)
	{
		cbyte = *(hex_in+i);
		if ( cbyte >= 'A') {
			cbyte = (cbyte - 'A') + 10;
		}			
		else {
			cbyte-= '0';
		}			
		data |= (cbyte << (4*(1-i)));
	}

	return data;
}