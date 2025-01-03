from __future__ import annotations

import collections.abc
import contextlib
import dataclasses
import enum
import functools
import json
import typing

import numpy as np
import typing_extensions

# This is a pedagogical implementation of utilities for (de-)serialization and
# validation using algebraic data types (ADT) as seen in the Rust library Serde (6K
# stars) and the Python library Pydantic (12K stars). The workflow shown below is not
# too different from classic (de-)serialization and validation libraries, except that it
# fully unleashes the power of expressiveness of ADT.
#
# ┌─────┐           ┌─────────┐                         ┌─────────────┐              ┌─────────┐           ┌─────┐
# │Bytes├──Decode──>│Raw Repr.├──Validate/Deserialize──>│Typed Context├──Serialize──>│Raw Repr.├──Encode──>│Bytes│
# └─────┘ ╷      ╷  └────┬────┘ ╷                    ╷  └─────────────┘ ╷            └─────────┘        ╷  └─────┘
#         └──────┘       │      └────────────────────┘                  └───────────────────────────────┘
#       json.loads()     │         serde.Validate()               json.dumps(..., default=serde.DefaultSerialize)
#                      dict/list/str/...
#
# It supports (de-)serialization and validation for data types of any combination of the
# following type constructors:
#   - Never, None, Union, tuple, Any
#   - bool, int, float, str
#   - Enum, Literal
#   - Sequence, MutableSequence, list, MutableSet, set, numpy.ndarray
#   - Mapping, MutableMapping, dict, TypedDict
#   - dataclass


TRANSPARENT = "__serde_transparent__"
T = typing.TypeVar("T")


def Validate(value: typing.Any, type_: typing.Type[T], allowTransform: bool) -> T:
    try:
        return _Validate(value, type_, allowTransform)  # type: ignore[no-any-return]
    except TypeError as e:
        with contextlib.suppress(TypeError):
            value = json.dumps(value)
        raise TypeError(f"{e}: value={value}")


@functools.cache
def GetTypeHints(ctor: typing.Any) -> dict[str, typing.Any]:
    return typing.get_type_hints(ctor)


def _ValidateNever(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    raise TypeError(f"{value!r} is not of type {type_}")


def _ValidateNone(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if value is None:
        return value
    raise TypeError(f"{value!r} is not of type {type(None)}")


def _ValidateBool(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, bool):
        return value
    raise TypeError(f"{value!r} is not of type {type_}")


def _ValidateInt(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, int) and not isinstance(value, bool):
        return value
    if isinstance(value, bool) and allowTransform:
        return int(value)
    if isinstance(value, str) and allowTransform:
        with contextlib.suppress(ValueError):
            return int(value)
    raise TypeError(f"{value!r} is not of type {type_}")


def _ValidateFloat(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, float):
        return value
    if isinstance(value, int) and allowTransform:
        return float(value)
    if isinstance(value, str) and allowTransform:
        with contextlib.suppress(ValueError):
            return float(value)
    raise TypeError(f"{value!r} is not of type {type_}")


def _ValidateStr(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, str):
        return value
    raise TypeError(f"{value!r} is not of type {type_}")


def _ValidateEnum(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, ctor):
        return value
    if allowTransform:
        with contextlib.suppress(ValueError):
            return ctor(value)
    raise TypeError(f"{value!r} cannot be coerced into type {type_}")


def _ValidateLiteral(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    for alternative in typeArgs:
        if type(alternative) is type(value) and alternative == value:
            return value
        if isinstance(enumT := type(alternative), enum.EnumMeta) and allowTransform:
            with contextlib.suppress(ValueError):
                if alternative == (value_ := enumT(value)):
                    return value_
    raise TypeError(f"{value!r} is not any of {' | '.join(map(repr, typeArgs))}")


def _ValidateSet(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if not isinstance(value, collections.abc.Sequence):
        raise TypeError(f"{value!r} is not of type {type_}")
    if len(typeArgs) != 1:
        raise TypeError(f"Expect 1 generic type argument for type {type_}")
    value_ = set(_Validate(v, typeArgs[0], allowTransform) for v in value)
    return value_ if allowTransform else value


def _ValidateNumpy(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, np.ndarray):
        return value
    if allowTransform:
        if not (not typeArgs or (len(typeArgs) == 2 and len(typing.get_args(typeArgs[1])) == 1)):
            # Numpy types are like: np.ndarray[Any, np.dtype[np.float64]]
            raise TypeError(f"Invalid type {type_}")
        with contextlib.suppress(ValueError):
            return (
                np.array(value, dtype=typing.get_args(typeArgs[1])[0])
                if typeArgs
                else np.array(value)  # fmt: skip
            )
    raise TypeError(f"{value!r} is not of type {type_}")


def _ValidateList(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if not isinstance(value, collections.abc.Sequence):
        raise TypeError(f"{value!r} is not of type {type_}")
    if len(typeArgs) != 1:
        raise TypeError(f"Expect 1 generic type argument for type {type_}")
    value_ = list(_Validate(v, typeArgs[0], allowTransform) for v in value)
    return value_ if allowTransform else value


def _ValidateDataclass(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if isinstance(value, ctor):
        return value
    if not isinstance(value, collections.abc.Mapping):
        raise TypeError(f"{value!r} is not of type {collections.abc.Mapping}")
    fields = dataclasses.fields(ctor)
    fieldValues: dict[str, typing.Any] = {}
    # We are not using field.type because SQLAlchemy screwed them up
    tdict = GetTypeHints(ctor)
    if fields_ := [f for f in fields if f.init and f.repr and f.metadata.get(TRANSPARENT, False)]:
        if len(fields_) > 1:
            raise TypeError(f"More than one transparent fields in dataclass {ctor}")
        for alternative in ((tdict[fields_[0].name],) + typeArgs):
            try:
                fieldValues[fields_[0].name] = _Validate(value, alternative, allowTransform)
                break
            except TypeError as e:
                errMsg += f"\n{alternative}: {e}" 
        if fields_[0].name not in fieldValues:
            raise TypeError(f"{value!r} is not of type {' | '.join(map(str, typeArgs))}{errMsg}")
        value_ = ctor(**fieldValues)
        return value_ if allowTransform else value
    if missingFields := [
        f.name
        for f in fields
        if f.init
        and f.name not in value
        and f.default is dataclasses.MISSING
        and f.default_factory is dataclasses.MISSING  # fmt: skip
    ]:
        raise TypeError(f"Fields {', '.join(map(repr, missingFields))} are missing in {value!r}")
    for f in fields:
        if f.init and f.name in value:
            errMsg = ""
            for alternative in ((tdict[f.name],) + typeArgs):
                try:
                    fieldValues[f.name] = _Validate(value[f.name], alternative, allowTransform)
                    break
                except TypeError as e:
                    errMsg += f"\n{alternative}: {e}" 
            if f.name not in fieldValues:
                raise TypeError(f"{value!r} is not of type {' | '.join(map(str, typeArgs))}{errMsg}")
    value_ = ctor(**fieldValues) 
    return value_ if allowTransform else value


def _ValidateTypedDict(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if not isinstance(value, collections.abc.Mapping):
        raise TypeError(f"{value!r} is not of type {type_}")
    tdict = GetTypeHints(ctor)
    optKeys = ctor.__optional_keys__
    if missingKeys := [k for k in tdict if k not in optKeys and k not in value]:
        raise TypeError(f"Keys {', '.join(map(repr, missingKeys))} are missing in {value!r}")
    value_ = dict(
        (k, _Validate(v, tdict[k], allowTransform)) if k in tdict else (k, v)
        for k, v in value.items()
    )  # fmt: skip
    return value_ if allowTransform else value


def _ValidateNewType(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    value = _Validate(value, ctor.__supertype__, allowTransform)
    return ctor.__validator__(value) if hasattr(ctor, "__validator__") else value


def _ValidateDict(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if not isinstance(value, collections.abc.Mapping):
        raise TypeError(f"{value!r} is not of type {type_}")
    if len(typeArgs) != 2:
        raise TypeError(f"Expect 2 generic type arguments for type {type_}")
    kType, vType = typeArgs
    value_ = dict(
        (_Validate(k, kType, allowTransform), _Validate(v, vType, allowTransform))
        for k, v in value.items()
    )  # fmt: skip
    return value_ if allowTransform else value


def _ValidateTuple(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    if not isinstance(value, collections.abc.Sequence):
        raise TypeError(f"{value!r} is not of type {type_}")
    if not typeArgs:
        raise TypeError(f"Expect generic type arguments for type {type_}")
    elif len(typeArgs) == 2 and typeArgs[1] is Ellipsis:
        value_ = tuple(_Validate(v, typeArgs[0], allowTransform) for v in value)
    elif len(value) == len(typeArgs):
        value_ = tuple(_Validate(v, t, allowTransform) for v, t in zip(value, typeArgs))
    else:
        raise TypeError(f"{value!r} is not in the shape specified by {type_}")
    return value_ if allowTransform else value


def _ValidateUnion(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    errMsg = ""
    for alternative in typeArgs:
        try:
            return _Validate(value, alternative, allowTransform)
        except TypeError as e:
            errMsg += f"\n{alternative}: {e}"
    raise TypeError(f"{value!r} is not of type {' | '.join(map(str, typeArgs))}{errMsg}")


def _ValidateAny(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    return value


def _ValidateRequired(value: typing.Any, type_: typing.Any, ctor: typing.Any, typeArgs: tuple[typing.Any, ...], allowTransform: bool) -> typing.Any:
    return _Validate(value, typeArgs[0], allowTransform)


HASHED_VALIDATORS = {
    typing_extensions.Never: _ValidateNever,
    None: _ValidateNone,
    type(None): _ValidateNone,
    bool: _ValidateBool,
    int: _ValidateInt,
    float: _ValidateFloat,
    str: _ValidateStr,
    typing.Literal: _ValidateLiteral,
    set: _ValidateSet,
    collections.abc.MutableSet: _ValidateSet,
    np.ndarray: _ValidateNumpy,
    list: _ValidateList,
    collections.abc.Sequence: _ValidateList,
    collections.abc.MutableSequence: _ValidateList,
    dict: _ValidateDict,
    collections.abc.Mapping: _ValidateDict,
    collections.abc.MutableMapping: _ValidateDict,
    tuple: _ValidateTuple,
    typing.Union: _ValidateUnion,
    typing.Any: _ValidateAny,
    typing_extensions.Required: _ValidateRequired,
}


def _Validate(value: typing.Any, type_: typing.Any, allowTransform: bool) -> typing.Any:
    assert not isinstance(type_, str)
    if ctor := typing.get_origin(type_):  # ctor: type constructor
        typeArgs = typing.get_args(type_)
    else:
        ctor, typeArgs = type_, ()

    with contextlib.suppress(KeyError):
        return HASHED_VALIDATORS[ctor](value, type_, ctor, typeArgs, allowTransform)

    if isinstance(ctor, enum.EnumMeta):
        return _ValidateEnum(value, type_, ctor, typeArgs, allowTransform)

    elif hasattr(ctor, "__supertype__"):  # NewType
        return _ValidateNewType(value, type_, ctor, typeArgs, allowTransform)

    elif dataclasses.is_dataclass(ctor):
        return _ValidateDataclass(value, type_, ctor, typeArgs, allowTransform)

    elif typing_extensions.is_typeddict(ctor):
        return _ValidateTypedDict(value, type_, ctor, typeArgs, allowTransform)

    else:
        raise TypeError(f"Do not know how to validate {value!r} against type {type_}")


HASHED_SERIALIZERS = {
    set: list,
    np.ndarray: lambda o: o.tolist(),
    tuple: list,
}


def DefaultSerialize(o: typing.Any) -> typing.Any:
    with contextlib.suppress(KeyError):
        return HASHED_SERIALIZERS[type(o)](o)  # type: ignore[no-untyped-call]

    if isinstance(type(o), enum.EnumMeta):
        return o.value

    elif dataclasses.is_dataclass(o):
        fields = dataclasses.fields(o.__class__)
        if fields_ := [f for f in fields if f.init and f.repr and f.metadata.get(TRANSPARENT, False)]:
            if len(fields_) > 1:
                raise TypeError(f"More than one transparent fields in dataclass {o.__class__.__name__}")
            return getattr(o, fields_[0].name)
        else:
            return {f.name: v for f in fields if f.repr and (v := getattr(o, f.name)) is not None}

    else:
        raise TypeError(f"Object of type {o.__class__.__name__} is not JSON serializable")


def Serialize(o: typing.Any) -> typing.Any:
    if o is None or isinstance(o, (bool, int, float, str)):
        return o
    elif isinstance(o, list):
        return list(Serialize(f) for f in o)
    elif isinstance(o, dict):
        return {name: Serialize(f) for name, f in o.items()}
    else:
        return Serialize(DefaultSerialize(o))


try:
    import re

    from sqlalchemy.orm import MappedAsDataclass, Mapper

    # `sqlalchemy.orm.util._cleanup_mapped_str_annotation` uses hacky string
    # manipulation and arbitrarily screws up forward reference expressions in
    # `__annotations__`. Here we introduce a special base class to guard and sanitize
    # type hints of mapped fields for runtime use.
    class FixMappedAnnotations:
        def __init_subclass__(cls) -> None:
            if MappedAsDataclass in cls.__mro__[: cls.__mro__.index(FixMappedAnnotations)]:
                raise TypeError("FixMappedAnnotations should be base class of MappedAsDataclass")

            annotations = cls.__annotations__
            super().__init_subclass__()  # dataclass_transform and all SQLAlchemy magics happen here
            cls.__annotations__ = {
                k: re.sub(r"^(?:(?:sqlalchemy\.)?orm\.)?Mapped\[(.*)]$", r"\1", v) if isinstance(v, str)
                else typing.get_args(v)[0] if typing.get_origin(v) is Mapper
                else v
                for k, v in annotations.items()  # fmt: skip
            }

except ImportError:
    pass
