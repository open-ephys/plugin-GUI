# set things up here
#global last=100

# this function is called once per buffer upddate and is passed
# the current buffer in data

#data[:]=sin(data/tscale)*200;


function oe_process!(data)
	
	global last;	

	f=0.05;
	for i in eachindex(data)
		
		data[i]=f*data[i] + (1-f)*last;
		last = data[i];
	end

end