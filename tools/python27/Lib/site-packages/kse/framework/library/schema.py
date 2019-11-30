'''
JSON schema extension
'''

import copy

from jsonschema import Draft4Validator, validators


def extend_with_default(validator_class):
    '''
    Extend validator with default processing
    '''
    validate_properties = validator_class.VALIDATORS['properties']

    def set_defaults(validator, properties, instance, schema):
        '''
        Set property value to default if not present
        '''
        for prop in properties:
            subschema = properties[prop]
            if 'default' in subschema:
                instance.setdefault(prop, copy.deepcopy(subschema['default']))

        for error in validate_properties(validator, properties, instance, schema):
            yield error

    return validators.extend(validator_class, {'properties': set_defaults})


DefaultValidatingDraft4Validator = extend_with_default(Draft4Validator)
