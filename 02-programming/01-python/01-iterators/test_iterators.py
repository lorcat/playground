"""
Class demonstrating two types of iterators: yield() and __iter__() based ones
"""

import numpy as np


class CustomIter:
    def __init__(self):
        self._container = np.linspace(-0.1, 0.1, 2 + 1)
        self._ci = 0

    def __iter__(self):
        return self

    def __next__(self):
        if self._ci < len(self._container):
            res = self._container[self._ci]
            self._ci += 1
        else:
            raise StopIteration

        return res

    def generator(self):
        for el in self._container:
            yield el


def main():
    ci = CustomIter()

    print("Example 01")
    for el in ci:
        print("{:6.4f}".format(el))

    print("\nExample 02")
    for el in ci.generator():
        print("{:6.4f}".format(el))

if __name__ == '__main__':
    main()
