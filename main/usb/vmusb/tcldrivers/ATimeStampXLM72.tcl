#===================================================================
# class ATimeStampXLM72
#===================================================================
itcl::class ATimeStampXLM72 {
	inherit AXLM72

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {}

	public method Clear {} {Write fpga 0 1; Write fpga 0 0}
	public method Init {}
	public method sReadStamp {stack}
}

itcl::body ATimeStampXLM72::Init {} {
	AccessBus 0x10000
	Clear
	ReleaseBus
}

itcl::body ATimeStampXLM72::sReadStamp {stack} {
	sAccessBus $stack 0x10000
	sRead $stack fpga 4
	sRead $stack fpga 8
	sReleaseBus $stack
}
