import random

def random_sources(n, len=100):
    sources = list(range(n))
    if n < len:
        return sources
    random.shuffle(sources)
    sources = sources[:len]
    sources.sort()
    return sources
