if not "x%PROCESSOR_IDENTIFIER:AMD=%"=="x%PROCESSOR_IDENTIFIER%" (
	set favor_arch=AMD64
) else if not "x%PROCESSOR_IDENTIFIER:INTEL=%"=="x%PROCESSOR_IDENTIFIER%" (
	set favor_arch=INTEL64
) else (
	echo can't detect cpu arch
	pause
)