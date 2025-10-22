def predict(hardware):
    performance = performance_model.predict(hardware)
    wattage = hardware['cpus']['wattage']

    return f'({performance}) / 1000 * {wattage}'