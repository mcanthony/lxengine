function fibonacci (int32 n) => int32
{
	print("fibonacci(${0})", n);
	
	if (n === 0) 
		return 0;
	else if (n === 1)
		return 1;
	else 
	{
		var (call1, call2) = .fork( fibonacci(n -1), fibonacci(n -2) );
		print("waiting (${0}, ${1})", n-1, n-2);		
		.wait(call1, call2);
		return call1.value + call2.value;
	}
}




	
