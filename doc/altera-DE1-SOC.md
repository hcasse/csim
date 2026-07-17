# NIOS II - DE1

## Parallel Ports

### Structure

00	DAR (data register)
04	DIR (direction register, 0 input, 1 output)
08	IMR (Interrupt Mask Register)
0C	ECR	(Edge Capture Register)
	write: reset

### LEDs

LEDR9-0
	0xFF200000<9..0>

### 7-Segment

bit map
	-0-
  5|   |1
    -6-
  4|   |2
    -3-

0xFF200020<30..24>	HEX3
0xFF200020<22..16>	HEX2
0xFF200020<14..8>	HEX3
0xFF200020<6..0>	HEX0

0xFF200020<14..8>	HEX5
0xFF200020<6..0>	HEX4


### Slider Switch

0xFF200040<9..0>
