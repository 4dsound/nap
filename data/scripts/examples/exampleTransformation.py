import nap


class Transformation:

    def __init__(self, entity, name, parameterManager):
        self.entity = entity
        self.name = name
        self.parameterManager = parameterManager

        self.amount = self.parameterManager.addParameterFloat("amount", 0.0, 0.0, 1.0, False)



    def apply(self, transform):
        transform.Position += nap.vec3(self.amount.Value, self.amount.Value, self.amount.Value)
        transform.Scale += nap.vec3(self.amount.Value, self.amount.Value, self.amount.Value)
        return transform

    def update(self, deltaTime):
        pass
