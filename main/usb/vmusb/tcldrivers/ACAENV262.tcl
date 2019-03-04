#===================================================================
# class ACAENV262
#===================================================================
itcl::class ACAENV262 {
	protected variable device
	private variable base
	private variable wecl
	private variable wnimlevel
	private variable wnimpulse
	private variable rnimlevel

	constructor {de ba} {
		set device $de
		set base $ba
		set wecl [expr $base+0x4]
		set wnimlevel [expr $base+0x6]
		set wnimpulse [expr $base+0x8]
		set rnimlevel [expr $base+0xa]
	}
	
	destructor {}
	
	public method GetVariable {v} {set $v}
	public method WriteECL {bits} {$device Write24 $wecl $bits}
	public method WriteNIMLevel {bits} {$device Write24 $wnimlevel $bits}
	public method WriteNIMPulse {bits} {$device Write24 $wnimpulse $bits}
	public method ReadNIMLevel {} {return [$device Read24 $rnimlevel]}
	public method sWriteECL {stack bits} {$device sWrite24 $stack $wecl $bits}
	public method sWriteNIMLevel {stack bits} {$device sWrite24 $stack $wnimlevel $bits}
	public method sWriteNIMPulse {stack bits} {$device sWrite24 $stack $wnimpulse $bits}
	public method sReadNIMLevel {stack} {$device sRead24 $stack $rnimlevel}
}
