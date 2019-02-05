struct TestStruct
{
	float3 InPosTest;
};

StructuredBuffer<TestStruct> MyTestStruct;

VSOut VSmain(VSIn input)
{
	VSOut output;
	output.Pos = MaterializeAttr(input);
	return output;
}
