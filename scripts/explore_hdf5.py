# small helper script to get hdf5 format information
import h5py
import argparse


def print_attributes(dataset, i):
    j = 1
    for key, val in dataset.attrs.items():
        print(i * " " + str(j) + ") " + str(key) + " -> " + str(val))
        j += 1

def print_dataset(dataset, i):
    if not hasattr(dataset, 'keys'):
        return

    for key in list(dataset.keys()):
        dset = dataset[key]
        shape =  str(dset.shape) if hasattr(dset, 'shape') else ""
        size = str(dset.size) if hasattr(dset, 'size') else ""
        dtype = str(dset.dtype) if hasattr(dset, 'dtype') else ""
        info =  " [" + shape + ", " + size + ", " + dtype + "]" if dtype else ""
        print(3*i*" " + "* D: " + key + info )
        print_attributes(dset, 3*i+1)
        print_dataset(dset, i+1)


parser = argparse.ArgumentParser()
parser.add_argument("fname")
args = parser.parse_args()

f = h5py.File(args.fname, 'r')
print(args.fname)
for key in list(f.keys()):
    print_dataset(f[key], 1)
