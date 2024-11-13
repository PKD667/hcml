# HCML-(X)

Introducing the HCML-(X) specification, that add new tools to the HCML specification.

## Basic HCML

### <?load>

The `<?load>` tag is used to load a file into the current file. The file is loaded as if it was written in the current file.

```hcml
<?load file="file.md">
```
The `<?load>` is replaced in the html structue by the root tag of the file.

### <?set>

The `<?set>` tag is used to set a variable in the current file. The variable is set as if it was written in the current file. It can set multiple variables at once.

```hcml
<?set name="name">
    ... value ...
</?set>
```

### <?get>

The `<?get>` tag is used to get a variable in the current file. The variable is get as if it was written in the current file.

```hcml
<?get name="name">
```

#### Field specific <?get>

The `<?get>` tag can also be used to get a field of a variable.

```hcml
<?get name="name">
    <field>
</?get>
```

In this case, the `<?get>` tag is replaced by the value of the field of the variable.

#### Multiple fields

The `<?get>` tag can also be used to get multiple fields of a variable.

```hcml
<?get name="name">
    <field1>
    <field2>
</?get>
```

In this case, the `<?get>` tag is replaced by the values of the fields of the variable.

Its can also be used to get multiple nested fields of a variable.

```hcml
<?get name="name">
    <field1>
        <field2>
    </field1>
</?get>
```

In this case, the `<?get>` tag is replaced by the value of the nested field of the variable alone.

## HCM(X)

Here is the list of the new tags that are added to the HCML specification for integration with the HTMX library.

### <?hx-on>

The `<?hx-on>` tag is used to handle the result of a parent HTMX request. The tag containes server code that is executed when the parent request is done.

```html
<div hx-post="/endpoint">
    <?hx-on>
        <?if>
            <?get>
                </name>
            </?get>
        </?if>
        <?then>
            <?return>
                <p>Welcome <?get></name></?get> </p>
            </?return>
        <?else>
            <?return>
                <p>Not logged in</p>
            </?return>
        </?then>
    <?/hx-on>
</div>
```

