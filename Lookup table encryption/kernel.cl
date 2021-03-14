
__kernel void task (__global uint16 *input1,
					__global uint16 *lookupTable,
					__global uint *output) {
	
	int id = get_global_id(0);

	uint16 chunk = input1[id];

	int16 specialCharactersMask = chunk < 65 || chunk > 90;

	chunk = chunk - 65;

	uint16 tableA = lookupTable[0];
	tableA = tableA - 65;
	uint16 tableB = lookupTable[1];
	tableB = tableB - 65;

	uint16 result = shuffle2(tableA, tableB, chunk);
	result = result + 65;

	chunk = chunk + 65;

	uint16 finalResult = select(result, chunk, specialCharactersMask);

	vstore16 (finalResult, id, output);

}
