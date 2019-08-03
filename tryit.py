import foo

v = foo.Vox()
v.set_freq(3)

f = open("F", "w")
g = open("G", "w")
h = open("H", "w")

for i in range(foo.SAMPLES_PER_SECOND):
	f.write("{0} {1}\n".format(i, v.sine()))
	g.write("{0} {1}\n".format(i, v.ramp()))
	# h.write("{0} {1}\n".format(i, v.triangle()))
	h.write("{0} {1}\n".format(i, v.square()))
	v.step()

